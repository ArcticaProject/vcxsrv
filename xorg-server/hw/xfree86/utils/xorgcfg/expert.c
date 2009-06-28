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
#include "xf86config.h"
#include "options.h"
#include "screen.h"
#include "vidmode.h"
#include "monitor-cfg.h"
#include <X11/Shell.h>
#include <X11/CompositeP.h>
#include <X11/Xaw/AsciiText.h>
#include <X11/Xaw/Box.h>
#include <X11/Xaw/Command.h>
#include <X11/Xaw/Form.h>
#include <X11/Xaw/Label.h>
#include <X11/Xaw/MenuButton.h>
#include <X11/Xaw/Paned.h>
#include <X11/Xaw/Panner.h>
#include <X11/Xaw/Porthole.h>
#include <X11/Xaw/SimpleMenu.h>
#include <X11/Xaw/SmeBSB.h>
#include <X11/Xaw/Toggle.h>
#include <X11/Xaw/Tree.h>
#include <ctype.h>

/*
 * Types
 */
typedef struct _TreeNode TreeNode;
typedef union _TreeData TreeData;
typedef void (*NodeDeleteFunc)(TreeNode*);
typedef void (*NodeUpdateFunc)(TreeNode*);

union _TreeData {
    struct {
	Widget text;
    } files;
    struct {
	Widget text;
	XF86LoadPtr load;
    } module;
    struct {
	Widget text;
	XF86ConfModesPtr modes;
    } modes;
    struct {
	Widget text, value;
	XF86ConfModeLinePtr modeline;
    } modeline;
    struct {
	Widget text, vendor, board, busid, driver;
	XF86ConfVideoAdaptorPtr video;
    } video;
    struct {
	Widget text;
	XF86ConfVideoPortPtr port;
    } port;
    struct {
	Widget text, vendor, model, width, height, hsync, vrefresh,
	       gammaRed, gammaGreen, gammaBlue;
	XF86ConfMonitorPtr monitor;
    } monitor;
    struct {
	Widget menu;
	XF86ConfModesLinkPtr modeslink;
    } modeslink;
    struct {
	Widget text, vendor, board, chipset, busid, card, driver, ramdac,
	       dacSpeed, videoRam, textClockFreq, biosBase, memBase, ioBase,
	       clockChip, devClock, chipId, chipRev, irq, screen;
	XF86ConfDevicePtr device;
    } device;
    struct {
	Widget text, defaultDepth, defaultBpp, defaultFbBpp,
	       monitor, device;
	XF86ConfScreenPtr screen;
    } screen;
    struct {
	Widget menu;
	XF86ConfAdaptorLinkPtr adaptorlink;
    } adaptorlink;
    struct {
	Widget viewport, c_virtual, depth, bpp, visual, weight, black, white;
	XF86ConfDisplayPtr display;
    } display;
    struct {
	Widget text;
	XF86ModePtr mode;
    } mode;
    struct {
	Widget text;
	XF86ConfInputPtr input;
    } input;
    struct {
	Widget text;
	XF86ConfLayoutPtr layout;
    } layout;
    struct {
	Widget menu, button, scrnum, adjx, adjy;
	XF86ConfScreenPtr screen;
	XF86ConfAdjacencyPtr adjacency;
    } adjacency;
    struct {
	Widget menu;
	XF86ConfInputrefPtr inputref;
    } inputref;
    struct {
	Widget text;
	XF86ConfVendorPtr vendor;
    } vendor;
    struct {
	Widget text;
	XF86ConfVendSubPtr vendsub;
    } vendsub;
    struct {
	Widget name, group, mode;
	XF86ConfDRIPtr dri;
    } dri;
    struct {
	Widget count, size, flags;
	XF86ConfBuffersPtr buffers;
    } buffers;
};

struct _TreeNode {
    Widget node, toggle, treeParent;
    TreeNode *parent, *child, *next;
    TreeData *data;
    NodeDeleteFunc destroy;
    NodeUpdateFunc update;
};

/*
 * Prototypes
 */
static Bool ExpertInitialize(void);
static TreeNode *NewNode(TreeNode*, Widget, Widget, Widget, TreeData*);
static void DeleteNode(TreeNode*);
static void DestroyCallback(Widget, XtPointer, XtPointer);
static void PannerCallback(Widget, XtPointer, XtPointer);
static void PortholeCallback(Widget, XtPointer, XtPointer);
static void ToggleCallback(Widget, XtPointer, XtPointer);
static void ToggleNode(TreeNode*, Bool);
static void ToggleNodeRecursive(TreeNode*);
static void OptionsCallback(Widget, XtPointer, XtPointer);
static void RelayoutTree(void);
static void PopdownCallback(Widget, XtPointer, XtPointer);
static void UpdateConfig(TreeNode*);
static void DestroyTree(TreeNode*);

static void CreateFiles(TreeNode*);
static void CreateFilesField(TreeNode*, char*, char*);
static void UpdateFiles(TreeNode*);

static void CreateFontPath(TreeNode*, char*);
static Widget CreateFontPathField(TreeNode*, char*, Bool);
static void FontPathChanged(TreeNode*);
static void NewFontPathCallback(Widget, XtPointer, XtPointer);
static void FontPathCallback(Widget, XtPointer, XtPointer);

static void CreateModulePath(TreeNode*, char*);
static Widget CreateModulePathField(TreeNode*, char*, Bool);
static void ModulePathChanged(TreeNode*);
static void NewModulePathCallback(Widget, XtPointer, XtPointer);

static void CreateModule(TreeNode*, XF86LoadPtr);
static void CreateModuleField(TreeNode*, Bool);
static void ModuleDestroy(TreeNode*);
static void NewModuleCallback(Widget, XtPointer, XtPointer);

static void CreateModes(TreeNode*, XF86ConfModesPtr);
static void CreateModesField(TreeNode*, Bool);
static void ModesDestroy(TreeNode*);
static void NewModesCallback(Widget, XtPointer, XtPointer);
static void CreateModesModeLine(TreeNode*, XF86ConfModeLinePtr);
static void ModesModeLineDestroy(TreeNode*);
static void NewModesModeLineCallback(Widget, XtPointer, XtPointer);

static void CreateModeLineField(TreeNode*, Bool, Bool);
static XF86ConfModeLinePtr ParseModeLine(char*, char*);

static void CreateVideoAdaptor(TreeNode*, XF86ConfVideoAdaptorPtr);
static void CreateVideoAdaptorField(TreeNode*, Bool);
static void VideoAdaptorDestroy(TreeNode*);
static void NewVideoAdaptorCallback(Widget, XtPointer, XtPointer);
static void VideoAdaptorUpdate(TreeNode*);
static void CreateVideoPort(TreeNode*, XF86ConfVideoPortPtr);
static void CreateVideoPortField(TreeNode*, Bool);
static void VideoPortDestroy(TreeNode*);
static void NewVideoPortCallback(Widget, XtPointer, XtPointer);

static void CreateMonitor(TreeNode*, XF86ConfMonitorPtr);
static void CreateMonitorField(TreeNode*, Bool);
static void MonitorDestroy(TreeNode*);
static void NewMonitorCallback(Widget, XtPointer, XtPointer);
static void MonitorUpdate(TreeNode*);
static void CreateMonitorModeLine(TreeNode*, XF86ConfModeLinePtr);
static void MonitorModeLineDestroy(TreeNode*);
static void NewMonitorModeLineCallback(Widget, XtPointer, XtPointer);
static void CreateMonitorModes(TreeNode*, XF86ConfModesLinkPtr);
static void CreateMonitorModesField(TreeNode*, Bool);
static void MonitorModesLinkDestroy(TreeNode*);
static void NewMonitorModesCallback(Widget, XtPointer, XtPointer);

static void CreateDevice(TreeNode*, XF86ConfDevicePtr);
static void CreateDeviceField(TreeNode*, Bool);
static void NewDeviceCallback(Widget, XtPointer, XtPointer);
static void DeviceDestroy(TreeNode*);
static void DeviceUpdate(TreeNode*);

static void CreateScreen(TreeNode*, XF86ConfScreenPtr);
static void CreateScreenField(TreeNode*, Bool);
static void NewScreenCallback(Widget, XtPointer, XtPointer);
static void ScreenDestroy(TreeNode*);
static void ScreenUpdate(TreeNode*);
static void CreateScreenAdaptor(TreeNode*, XF86ConfAdaptorLinkPtr);
static void CreateScreenAdaptorField(TreeNode*, Bool);
static void NewScreenAdaptorCallback(Widget, XtPointer, XtPointer);
static void ScreenAdaptorDestroy(TreeNode*);
static void CreateScreenDisplay(TreeNode*, XF86ConfDisplayPtr);
static void CreateScreenDisplayField(TreeNode*, Bool);
static void NewScreenDisplayCallback(Widget, XtPointer, XtPointer);
static void ScreenDisplayDestroy(TreeNode*);
static void ScreenDisplayUpdate(TreeNode*);
static void CreateDisplayMode(TreeNode*, XF86ModePtr);
static void CreateDisplayModeField(TreeNode*, Bool);
static void NewDisplayModeCallback(Widget, XtPointer, XtPointer);
static void DisplayModeDestroy(TreeNode*);

static void CreateInput(TreeNode*, XF86ConfInputPtr);
static void CreateInputField(TreeNode*, Bool);
static void InputDestroy(TreeNode*);
static void NewInputCallback(Widget, XtPointer, XtPointer);
static void InputUpdate(TreeNode*);

static void CreateLayout(TreeNode*, XF86ConfLayoutPtr);
static void CreateLayoutField(TreeNode*, Bool);
static void LayoutDestroy(TreeNode*);
static void NewLayoutCallback(Widget, XtPointer, XtPointer);
static void CreateAdjacency(TreeNode*, XF86ConfAdjacencyPtr);
static void CreateAdjacencyField(TreeNode*, Bool);
static void AdjacencyDestroy(TreeNode*);
static void NewAdjacencyCallback(Widget, XtPointer, XtPointer);
static void AdjacencyMenuCallback(Widget, XtPointer, XtPointer);
static void AdjacencyToggleCallback(Widget, XtPointer, XtPointer);
static void CreateInputref(TreeNode*, XF86ConfInputrefPtr);
static void CreateInputrefField(TreeNode*, Bool);
static void InputrefDestroy(TreeNode*);
static void NewInputrefCallback(Widget, XtPointer, XtPointer);

static void CreateVendor(TreeNode*, XF86ConfVendorPtr);
static void CreateVendorField(TreeNode*, Bool);
static void VendorDestroy(TreeNode*);
static void NewVendorCallback(Widget, XtPointer, XtPointer);
static void CreateVendorSub(TreeNode*, XF86ConfVendSubPtr);
static void CreateVendorSubField(TreeNode*, Bool);
static void NewVendorSubCallback(Widget, XtPointer, XtPointer);
static void VendorSubDestroy(TreeNode*);
static void VendorSubUpdate(TreeNode*);

static void CreateDRI(TreeNode*, XF86ConfDRIPtr);
static void CreateDRIField(TreeNode*);
static void DRIUpdate(TreeNode*);

static void CreateBuffers(TreeNode*, XF86ConfBuffersPtr);
static void CreateBuffersField(TreeNode*, Bool);
static void BuffersDestroy(TreeNode*);
static void NewBuffersCallback(Widget, XtPointer, XtPointer);
static void BuffersUpdate(TreeNode*);

extern void RemoveDeviceCallback(Widget, XtPointer, XtPointer);

/* interface.c */
extern void InitializeDevices(void);
extern void SelectLayoutCallback(Widget, XtPointer, XtPointer);
extern void UpdateMenuDeviceList(int);
extern void SetConfigModeCallback(Widget, XtPointer, XtPointer);
extern void DefaultLayoutCallback(Widget, XtPointer, XtPointer);
extern void RemoveLayoutCallback(Widget, XtPointer, XtPointer);

/*
 * Initialization
 */
static Widget shell, expert, tree, panner;
extern Widget work, optionsShell, config, layoutp, topMenu;
extern xf86cfgDevice cpu_device;
static TreeNode *mainNode, *monitorTree, *screenTree, *layoutTree;

/*
 * Implementation
 */
void
ExpertConfigureStart(void)
{
    ExpertInitialize();

    XtPopup(shell, XtGrabExclusive);
    if (optionsShell == NULL)
	CreateOptionsShell();
    XtVaSetValues(optionsShell, XtNtransientFor, shell, NULL);
}

void
ExpertConfigureEnd(void)
{
    int i, save_config_mode = config_mode;
    Widget sme, layopt, layoutsme = NULL;
    XF86ConfLayoutPtr lay;

    XtVaSetValues(optionsShell, XtNtransientFor, toplevel, NULL);
    XtPopdown(shell);

    /* Need to do this to avoid all code elsewhere needing to update the
     * "expert" widget tree
     */
    UpdateConfig(mainNode);
    DestroyTree(mainNode);
    XtDestroyWidget(shell);
    expert = NULL;

    if (save_config_mode != CONFIG_LAYOUT)
	SetConfigModeCallback(topMenu, (XtPointer)CONFIG_LAYOUT, NULL);

    /* Reset everything as the "expert" interface can do almost anything
     * to the configuration.
     */
    for (i = 0; i < computer.num_screens; i++) {
	XtDestroyWidget(computer.screens[i]->widget);
	XtFree((XtPointer)computer.screens[i]);
    }
    XtFree((XtPointer)computer.screens);
    computer.screens = NULL;
    computer.num_screens = 0;

    for (i = 0; i < computer.num_devices; i++) {
	XtDestroyWidget(computer.devices[i]->widget);
	XtFree((XtPointer)computer.devices[i]);
    }
    XtFree((XtPointer)computer.devices);
    computer.devices = NULL;
    computer.num_devices = 0;

    for (i = 0; i < computer.num_layouts; i++) {
	XtFree((XtPointer)computer.layouts[i]->position);
	XtFree((XtPointer)computer.layouts[i]);
    }
    XtFree((XtPointer)computer.layouts);
    computer.layouts = NULL;
    computer.num_layouts = 0;

    for (i = 0; i < computer.num_vidmodes; i++)
	XtFree((XtPointer)computer.vidmodes[i]);
    XtFree((XtPointer)computer.vidmodes);
    computer.vidmodes = NULL;
    computer.num_vidmodes = 0;

    /* Reinitialize devices/screens */
    InitializeDevices();
    UpdateMenuDeviceList(MOUSE);
    UpdateMenuDeviceList(KEYBOARD);
    UpdateMenuDeviceList(CARD);
    UpdateMenuDeviceList(MONITOR);

    /* Update layout menu */
	/* first entry is "New server layout" */
    for (i = 1; i < ((CompositeWidget)layoutp)->composite.num_children; i++)
	XtDestroyWidget(((CompositeWidget)layoutp)->composite.children[i]);
    for (i = 0; i < layoutp->core.num_popups; i++)
	XtDestroyWidget(layoutp->core.popup_list[i]);
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
    computer.layout = NULL;
    SelectLayoutCallback(layoutsme,
			 XF86Config->conf_layout_lst, NULL);


    if (XF86Config->conf_flags && XF86Config->conf_flags->flg_option_lst)
	SetTip(&cpu_device);
    for (i = 0; i < computer.num_devices; i++)
	SetTip(computer.devices[i]);

    /* Reinitialize vidmodes */
    InitializeVidmodes();

    if (save_config_mode != CONFIG_LAYOUT)
	SetConfigModeCallback(topMenu, (XtPointer)(long)save_config_mode, NULL);
}

/*ARGSUSED*/
void
ExpertCloseAction(Widget w, XEvent *event, String *params, Cardinal *num_params)
{
    ExpertConfigureEnd();
}

/*ARGSUSED*/
void
ExpertCallback(Widget w, XtPointer user_data, XtPointer call_data)
{
    ExpertConfigureStart();
}

/*ARGSUSED*/
static void
PopdownCallback(Widget w, XtPointer user_data, XtPointer call_data)
{
    ExpertConfigureEnd();
}

/* Files */
static void
CreateFiles(TreeNode *files)
{
    XF86ConfFilesPtr file = XF86Config->conf_files;
    TreeNode *node, *fontpath, *modulepath;
    Widget w;
    char *value;

    value = file->file_logfile ? file->file_logfile : "";
    node = NewNode(files, NULL, NULL, files->node, 
		   (TreeData*)XtCalloc(1, sizeof(TreeData)));
    CreateFilesField(node, "LogFile", value);
    files->child = node;
    files->update = UpdateFiles;

    w = XtVaCreateManagedWidget("ModulePath", toggleWidgetClass, tree,
				XtNtreeParent, files->node, NULL);
    node->next = modulepath = NewNode(files, w, w, files->node, NULL);
    node = node->next;
    CreateModulePath(modulepath, NULL);

    w = XtVaCreateManagedWidget("FontPath", toggleWidgetClass, tree,
				XtNtreeParent, files->node, NULL);
    node->next = fontpath = NewNode(files, w, w, files->node, NULL);
    node = node->next;
    CreateFontPath(fontpath, NULL);
}

static void
CreateFilesField(TreeNode *node, char *name, char *value)
{
    Widget box, text;

    box = XtVaCreateManagedWidget(name, boxWidgetClass, tree,
				  XtNtreeParent, node->node, NULL);
    node->node = box;
    (void) XtVaCreateManagedWidget("label", labelWidgetClass, box,
				    XtNlabel, name, NULL);
    text = XtVaCreateManagedWidget("value", asciiTextWidgetClass, box,
				   XtNeditType, XawtextEdit, XtNstring, value,
				   NULL);
    node->data->files.text = text;
}

static void
UpdateFiles(TreeNode *files)
{
    char *str;

    /* LogFile */
    files = files->child;
    XtVaGetValues(files->data->files.text, XtNstring, &str, NULL);
    XtFree(XF86Config->conf_files->file_logfile);
    if (*str)
	XF86Config->conf_files->file_logfile = XtNewString(str);
    else
	XF86Config->conf_files->file_logfile = NULL;
}

/* FontPath */
/* Don't need to set the update tree field, as it is already set
 * as the destroy field */
static void
CreateFontPath(TreeNode *fontpath, char *path)
{
    TreeNode *prev = NULL, *node;

    if (path == NULL) {
	if (XF86Font_path) {
	    path = XtNewString(XF86Font_path);
	    if (XF86Config->conf_files && XF86Config->conf_files->file_fontpath) {
		XtFree(XF86Config->conf_files->file_fontpath);
		XF86Config->conf_files->file_fontpath = XtNewString(path);
	    }
	}
	else if (XF86Config->conf_files && XF86Config->conf_files->file_fontpath)
	    path = XtNewString(XF86Config->conf_files->file_fontpath);
    }
    else {
	path = XtNewString(path);
	if ((prev = fontpath->child) != NULL)
	    while (prev->next)
		prev = prev->next;
    }

    if (path) {
	char *s;

	for (s = strtok(path, ","); s != NULL; s = strtok(NULL, ",")) {
	    node = NewNode(fontpath, NULL, NULL, fontpath->node, NULL);
	    node->destroy = FontPathChanged;
	    (void) CreateFontPathField(node, s, False);
	    if (fontpath->child == NULL)
		fontpath->child = node;
	    else
		prev->next = node;
	    prev = node;
	}
	XtFree(path);
    }

    node = NewNode(fontpath, NULL, NULL, fontpath->node, NULL);
    (void) CreateFontPathField(node, "", True);
    if (fontpath->child == NULL)
	fontpath->child = node;
    else
	prev->next = node;
}

static Widget
CreateFontPathField(TreeNode *fontpath, char *value, Bool addnew)
{
    Widget box, command, text;
    TreeData *data;

    box = XtVaCreateWidget("fontpath", formWidgetClass, tree,
			   XtNtreeParent, fontpath->treeParent, NULL);
    fontpath->node = box;
    if (!addnew) {
	command = XtCreateManagedWidget("remove", commandWidgetClass, box,
					NULL, 0);
	XtAddCallback(command, XtNcallback, DestroyCallback,
		      (XtPointer)fontpath);
	command = XtCreateManagedWidget("up", commandWidgetClass, box, NULL, 0);
	XtAddCallback(command, XtNcallback, FontPathCallback,
		      (XtPointer)fontpath);
	command = XtCreateManagedWidget("down", commandWidgetClass, box, NULL, 0);
	XtAddCallback(command, XtNcallback, FontPathCallback,
		      (XtPointer)fontpath);
	text = XtVaCreateManagedWidget("value", asciiTextWidgetClass, box,
				       XtNeditType, XawtextEdit,
				       XtNstring, value, NULL);
    }
    else {
	command = XtCreateManagedWidget("new", commandWidgetClass, box, NULL, 0);
	XtAddCallback(command, XtNcallback, NewFontPathCallback, 
		      (XtPointer)fontpath);
	text = XtVaCreateManagedWidget("valueNew", asciiTextWidgetClass, box,
				       XtNeditType, XawtextEdit,
				       XtNstring, value, NULL);
    }
    data = (TreeData*)XtCalloc(1, sizeof(TreeData));
    data->files.text = text;
    fontpath->data = data;

    if (fontpath->treeParent && XtIsRealized(fontpath->treeParent))
	XtRealizeWidget(box);
    XtManageChild(box);

    return (box);
}

static void
FontPathChanged(TreeNode *node)
{
    TreeNode *parent = node->parent;
    char *fontpath = NULL, *str;
    Arg args[1];
    int pos = 0, len;

    /* last node is the "new" */
    for (node = parent->child; node->next != NULL; node = node->next) {
	if (pos)
	    fontpath[pos++] = ',';
	XtSetArg(args[0], XtNstring, &str);
	XtGetValues(node->data->files.text, args, 1);
	len = strlen(str) + 2;
	fontpath = XtRealloc(fontpath, pos + len);
	strcpy(fontpath + pos, str);
	pos += len - 2;
    }

    if (XF86Config->conf_files->file_fontpath)
	XtFree(XF86Config->conf_files->file_fontpath);
    XF86Config->conf_files->file_fontpath = fontpath;
}

static void
NewFontPathCallback(Widget unused, XtPointer user_data, XtPointer call_data)
{
    TreeNode *fontpath, *node = (TreeNode*)user_data;
    Arg args[1];
    char *str;

    XtSetArg(args[0], XtNstring, &str);
    XtGetValues(node->data->files.text, args, 1);
    if (*str == '\0')
	return;

    fontpath = node->parent;
    DeleteNode(node);
    CreateFontPath(fontpath, str);

    FontPathChanged(fontpath->child);
    RelayoutTree();
}

/*ARGSUSED*/
static void
FontPathCallback(Widget w, XtPointer user_data, XtPointer call_data)
{
    TreeNode *parent, *node, *fontpath = (TreeNode*)user_data;
    char *t1, *t2;
    Widget w1, w2;

    parent = fontpath->parent;
    node = parent->child;
    if (!node->next->next)
	return;
    if (strcmp(XtName(w), "up") == 0) {
	if (node == fontpath)
	    while (node->next->next)
		node = node->next;
	else
	    while (node && node->next != fontpath)
		node = node->next;
    }
    else {
	if (fontpath->next->next)
	    node = fontpath->next;
	/* else is already correct */
    }

    w1 = node->data->files.text;
    w2 = fontpath->data->files.text;

    XtVaGetValues(w1, XtNstring, &t1, NULL);
    XtVaGetValues(w2, XtNstring, &t2, NULL);
    t1 = XtNewString(t1);
    XtVaSetValues(w1, XtNstring, t2, NULL);
    XtVaSetValues(w2, XtNstring, t1, NULL);
    XtFree(t1);
}


/* ModulePath */
/* Don't need to set the update tree field, as it is already set
 * as the destroy field */
static void
CreateModulePath(TreeNode *modulepath, char *path)
{
    TreeNode *prev = NULL, *node;

    if (path == NULL) {
	if (XF86Module_path) {
	    path = XtNewString(XF86Module_path);
	    if (XF86Config->conf_files && XF86Config->conf_files->file_modulepath) {
		XtFree(XF86Config->conf_files->file_modulepath);
		XF86Config->conf_files->file_modulepath = XtNewString(path);
	    }
	}
	else if (XF86Config->conf_files && XF86Config->conf_files->file_modulepath)
	    path = XtNewString(XF86Config->conf_files->file_modulepath);
    }
    else {
	path = XtNewString(path);
	if ((prev = modulepath->child) != NULL)
	    while (prev->next)
		prev = prev->next;
    }

    if (path) {
	char *s;

	for (s = strtok(path, ","); s != NULL; s = strtok(NULL, ",")) {
	    node = NewNode(modulepath, NULL, NULL, modulepath->node, NULL);
	    node->destroy = ModulePathChanged;
	    (void) CreateModulePathField(node, s, False);
	    if (modulepath->child == NULL)
		modulepath->child = node;
	    else
		prev->next = node;
	    prev = node;
	}
	XtFree(path);
    }

    node = NewNode(modulepath, NULL, NULL, modulepath->node, NULL);
    (void) CreateModulePathField(node, "", True);
    if (modulepath->child == NULL)
	modulepath->child = node;
    else
	prev->next = node;
}

static Widget
CreateModulePathField(TreeNode *modulepath, char *value, Bool addnew)
{
    Widget box, command, text;
    TreeData *data;

    box = XtVaCreateWidget("modulepath", formWidgetClass, tree,
			   XtNtreeParent, modulepath->treeParent, NULL);
    modulepath->node = box;
    if (!addnew) {
	command = XtCreateManagedWidget("remove", commandWidgetClass, box,
					NULL, 0);
	XtAddCallback(command, XtNcallback, DestroyCallback,
		      (XtPointer)modulepath);
	text = XtVaCreateManagedWidget("value", asciiTextWidgetClass, box,
				       XtNeditType, XawtextEdit,
				       XtNstring, value, NULL);
    }
    else {
	command = XtCreateManagedWidget("new", commandWidgetClass, box, NULL, 0);
	XtAddCallback(command, XtNcallback, NewModulePathCallback, 
		      (XtPointer)modulepath);
	text = XtVaCreateManagedWidget("valueNew", asciiTextWidgetClass, box,
				       XtNeditType, XawtextEdit,
				       XtNstring, value, NULL);
    }
    data = (TreeData*)XtCalloc(1, sizeof(TreeData));
    data->files.text = text;
    modulepath->data = data;

    if (modulepath->treeParent && XtIsRealized(modulepath->treeParent))
	XtRealizeWidget(box);
    XtManageChild(box);

    return (box);
}

static void
ModulePathChanged(TreeNode *node)
{
    TreeNode *parent = node->parent;
    char *modulepath = NULL, *str;
    Arg args[1];
    int pos = 0, len;

    /* last node is the "new" */
    for (node = parent->child; node->next != NULL; node = node->next) {
	if (pos)
	    modulepath[pos++] = ',';
	XtSetArg(args[0], XtNstring, &str);
	XtGetValues(node->data->files.text, args, 1);
	len = strlen(str) + 2;
	modulepath = XtRealloc(modulepath, pos + len);
	strcpy(modulepath + pos, str);
	pos += len - 2;
    }

    if (XF86Config->conf_files->file_modulepath)
	XtFree(XF86Config->conf_files->file_modulepath);
    XF86Config->conf_files->file_modulepath = modulepath;
}

static void
NewModulePathCallback(Widget unused, XtPointer user_data, XtPointer call_data)
{
    TreeNode *modulepath, *node = (TreeNode*)user_data;
    Arg args[1];
    char *str;

    XtSetArg(args[0], XtNstring, &str);
    XtGetValues(node->data->files.text, args, 1);
    if (*str == '\0')
	return;

    modulepath = node->parent;
    DeleteNode(node);
    CreateModulePath(modulepath, str);

    ModulePathChanged(modulepath->child);
    RelayoutTree();
}

/* Module */
static void
CreateModule(TreeNode *module, XF86LoadPtr load)
{
    TreeNode *prev, *node;
    TreeData *data;

    if ((prev = module->child) != NULL)
	while (prev->next)
	    prev = prev->next;

    while (load) {
	data = (TreeData*)XtCalloc(1, sizeof(TreeData));
	data->module.load = load;
	node = NewNode(module, NULL, NULL, module->node, data);
	node->destroy = ModuleDestroy;
	CreateModuleField(node, False);
	if (module->child == NULL)
	    module->child = node;
	else
	    prev->next = node;
	prev = node;
	load = (XF86LoadPtr)(load->list.next);
    }

    data = (TreeData*)XtCalloc(1, sizeof(TreeData));
    node = NewNode(module, NULL, NULL, module->node, data);
    CreateModuleField(node, True);
    if (module->child == NULL)
	module->child = node;
    else
	prev->next = node;
}

static void
CreateModuleField(TreeNode *node, Bool addnew)
{
    Widget box, command, label;

    box = XtVaCreateWidget("module", formWidgetClass, tree,
			   XtNtreeParent, node->treeParent, NULL);
    node->node = box;

    if (!addnew) {
	XF86OptionPtr *options;
	XF86LoadPtr load = node->data->module.load;

	options = &(load->load_opt);
	command = XtCreateManagedWidget("remove", commandWidgetClass, box,
					NULL, 0);
	XtAddCallback(command, XtNcallback, DestroyCallback, (XtPointer)node);
	command = XtCreateManagedWidget("options", commandWidgetClass, box,
					NULL, 0);
	XtAddCallback(command, XtNcallback, OptionsCallback, (XtPointer)options);
	label = XtVaCreateManagedWidget("label", labelWidgetClass, box,
					XtNlabel, load->load_name, NULL);
    }
    else {
	command = XtCreateManagedWidget("new", commandWidgetClass, box,
					NULL, 0);
	XtAddCallback(command, XtNcallback, NewModuleCallback, (XtPointer)node);
	label = XtVaCreateManagedWidget("value", asciiTextWidgetClass, box,
					XtNeditType, XawtextEdit,
					NULL);
	node->data->module.text = label;
    }
    if (XtIsRealized(node->treeParent))
	XtRealizeWidget(box);
    XtManageChild(box);
}

/*ARGUSED*/
static void
ModuleDestroy(TreeNode *node)
{
    if (node->data->module.load)
	xf86removeModule(XF86Config, node->data->module.load);
}

/*ARGSUSED*/
static void
NewModuleCallback(Widget unused, XtPointer user_data, XtPointer call_data)
{
    TreeNode *module, *node = (TreeNode*)user_data;
    XF86LoadPtr load;
    Arg args[1];
    char *label;

    XtSetArg(args[0], XtNstring, &label);
    XtGetValues(node->data->module.text, args, 1);
    if (*label == '\0')
	return;

    module = node->parent;
    DeleteNode(node);
    load = (XF86LoadPtr)XtCalloc(1, sizeof(XF86LoadRec));
    load->load_name = XtNewString(label);
    XF86Config->conf_modules->mod_load_lst =
	xf86addModule(XF86Config->conf_modules->mod_load_lst, load);

    CreateModule(module, load);
    RelayoutTree();
}

/* Modes */
static void
CreateModes(TreeNode *parent, XF86ConfModesPtr modes)
{
    TreeNode *node, *prev;
    TreeData *data;

    if ((prev = parent->child) != NULL)
	while (prev->next)
	    prev = prev->next;

    while (modes) {
	data = (TreeData*)XtCalloc(1, sizeof(TreeData));
	data->modes.modes = modes;
	node = NewNode(parent, NULL, NULL, parent->node, data);
	node->destroy = ModesDestroy;
	CreateModesField(node, False);
	if (parent->child == NULL)
	    parent->child = node;
	else
	    prev->next = node;
	prev = node;

	modes = (XF86ConfModesPtr)(modes->list.next);
    }

    data = (TreeData*)XtCalloc(1, sizeof(TreeData));
    node = NewNode(parent, NULL, NULL, parent->node, data);
    CreateModesField(node, True);
    if (parent->child == NULL)
	parent->child = node;
    else
	prev->next = node;
}

static void
CreateModesField(TreeNode *node, Bool addnew)
{
    Widget box, command, label;

    box = XtVaCreateWidget("modes", formWidgetClass, tree,
			   XtNtreeParent, node->treeParent, NULL);
    node->node = box;

    if (!addnew) {
	XF86ConfModesPtr modes = node->data->modes.modes;

	command = XtCreateManagedWidget("remove", commandWidgetClass, box,
					NULL, 0);
	XtAddCallback(command, XtNcallback, DestroyCallback, (XtPointer)node);
	label = XtVaCreateManagedWidget("mode", toggleWidgetClass, box,
					XtNlabel, modes->modes_identifier,
					XtNstate, True,
					NULL);
	node->toggle = label;
	XtAddCallback(label, XtNcallback, ToggleCallback, (XtPointer)node);
	CreateModesModeLine(node, node->data->modes.modes->mon_modeline_lst);
    }
    else {
	command = XtCreateManagedWidget("new", commandWidgetClass, box,
					NULL, 0);
	XtAddCallback(command, XtNcallback, NewModesCallback, (XtPointer)node);
	label = XtVaCreateManagedWidget("value", asciiTextWidgetClass, box,
					XtNeditType, XawtextEdit,
					NULL);
	node->data->modes.text = label;
    }
    if (XtIsRealized(node->treeParent))
	XtRealizeWidget(box);
    XtManageChild(box);
}

/*ARGUSED*/
static void
ModesDestroy(TreeNode *node)
{
    if (node->data->modes.modes) {
	int i;
	TreeNode *mon = monitorTree->child;

	/* last one is the "new" entry */
	while (mon && mon->next) {
	    /* UseModes is the second entry */
	    TreeNode *mod = mon->child->next->child;
	    CompositeWidget composite;

	    while (mod && mod->next) {
		TreeNode *next = mod->next;

		if (mod && strcmp(mod->data->modeslink.modeslink->ml_modes_str,
				  node->data->modes.modes->modes_identifier) == 0)
		    /* Needs to do string comparison because may be deleting
		     * a "test" Modes section, with no Modelines.
		     */
		    DeleteNode(mod);
		mod = next;
	    }
	    composite = (CompositeWidget)mod->data->modeslink.menu;

	    for (i = 0; i < composite->composite.num_children; ++i)
		if (strcmp(XtName(composite->composite.children[i]),
			   node->data->modes.modes->modes_identifier) == 0)
		    XtDestroyWidget(composite->composite.children[i]);

	    mon = mon->next;
	}

	xf86removeModes(XF86Config, node->data->modes.modes);
    }
}

/*ARGSUSED*/
static void
NewModesCallback(Widget unused, XtPointer user_data, XtPointer call_data)
{
    TreeNode *parent, *node = (TreeNode*)user_data;
    XF86ConfModesPtr modes;
    Arg args[1];
    char *label;

    XtSetArg(args[0], XtNstring, &label);
    XtGetValues(node->data->modes.text, args, 1);
    if (*label == '\0')
	return;

    parent = node->parent;
    DeleteNode(node);
    modes = (XF86ConfModesPtr)XtCalloc(1, sizeof(XF86ConfModesRec));
    modes->modes_identifier = XtNewString(label);
    XF86Config->conf_modes_lst =
	xf86addModes(XF86Config->conf_modes_lst, modes);

    {
	TreeNode *mon = monitorTree->child;
	Widget sme;

	/* last one is the "new" entry */
	while (mon && mon->next) {
	    /* UseModes is the second entry */
	    TreeNode *mod = mon->child->next->child;

	    while (mod && mod->next)
		mod = mod->next;

	    sme = XtCreateManagedWidget(modes->modes_identifier,
					smeBSBObjectClass,
					mod->data->modeslink.menu, NULL, 0);
	    XtAddCallback(sme, XtNcallback, NewMonitorModesCallback,
			  (XtPointer)mod);

	    mon = mon->next;
	}
    }

    CreateModes(parent, modes);
    RelayoutTree();
}

static void
CreateModesModeLine(TreeNode *parent, XF86ConfModeLinePtr modeline)
{
    TreeNode *node, *prev;
    TreeData *data;

    if ((prev = parent->child) != NULL)
	while (prev->next)
	    prev = prev->next;

    while (modeline) {
	data = (TreeData*)XtCalloc(1, sizeof(TreeData));
	data->modeline.modeline = modeline;
	node = NewNode(parent, NULL, NULL, parent->node, data);
	node->destroy = ModesModeLineDestroy;
	CreateModeLineField(node, False, False);
	if (parent->child == NULL)
	    parent->child = node;
	else
	    prev->next = node;
	prev = node;
	modeline = (XF86ConfModeLinePtr)(modeline->list.next);
    }
    data = (TreeData*)XtCalloc(1, sizeof(TreeData));
    node = NewNode(parent, NULL, NULL, parent->node, data);
    if (parent->child == NULL)
	parent->child = node;
    else
	prev->next = node;
    prev = node;
    CreateModeLineField(node, True, False);
}

/* This function should allow creating modelines for the
   Mode and Monitor section */
static void
CreateModeLineField(TreeNode *node, Bool addnew, Bool monitor)
{
    Widget box, command;
    char buf[512], tmp[32];

    box = XtVaCreateWidget("modeline", formWidgetClass, tree,
			   XtNtreeParent, node->treeParent, NULL);
    node->node = box;

    if (!addnew) {
	XF86ConfModeLinePtr mod = node->data->modeline.modeline;

	command = XtCreateManagedWidget("remove", commandWidgetClass,
					box, NULL, 0);
	XtAddCallback(command, XtNcallback, DestroyCallback, (XtPointer)node);
	XtVaCreateManagedWidget("label", labelWidgetClass, box,
				XtNlabel, mod->ml_identifier, NULL);

	XmuSnprintf(buf, sizeof(buf), "%g %d %d %d %d %d %d %d %d",
	    mod->ml_clock / 1000., mod->ml_hdisplay, mod->ml_hsyncstart,
	    mod->ml_hsyncend, mod->ml_htotal, mod->ml_vdisplay,
	    mod->ml_vsyncstart, mod->ml_vsyncend, mod->ml_vtotal);
	if (mod->ml_flags & XF86CONF_INTERLACE)
	    strcat(buf, " interlace");
	if (mod->ml_flags & XF86CONF_PHSYNC)
	    strcat(buf, " +hsync");
	if (mod->ml_flags & XF86CONF_NHSYNC)
	    strcat(buf, " -hsync");
	if (mod->ml_flags & XF86CONF_PVSYNC)
	    strcat(buf, " +vsync");
	if (mod->ml_flags & XF86CONF_NVSYNC)
	    strcat(buf, " -vsync");
	if (mod->ml_flags & XF86CONF_CSYNC)
	    strcat(buf, " composite");
	if (mod->ml_flags & XF86CONF_PCSYNC)
	    strcat(buf, " +csync");
	if (mod->ml_flags & XF86CONF_NCSYNC)
	    strcat(buf, " -csync");
	if (mod->ml_flags & XF86CONF_DBLSCAN)
	    strcat(buf, " doublescan");
	if (mod->ml_flags & XF86CONF_BCAST)
	    strcat(buf, " bcast");
	if (mod->ml_flags & XF86CONF_HSKEW) {
	    XmuSnprintf(tmp, sizeof(tmp), " hskew %d", mod->ml_hskew);
	    strcat(buf, tmp);
	}
	if (mod->ml_flags & XF86CONF_VSCAN) {
	    XmuSnprintf(tmp, sizeof(tmp), " vscan %d", mod->ml_vscan);
	    strcat(buf, tmp);
	}
	if (mod->ml_flags & XF86CONF_CUSTOM)
	    strcat(buf, " custom");
	node->data->modeline.value =
	    XtVaCreateManagedWidget("modeline", asciiTextWidgetClass, box,
				    XtNeditType, XawtextEdit,
				    XtNstring, buf, NULL);
    }
    else {
	*buf = '\0';
	command = XtCreateManagedWidget("new", commandWidgetClass,
					box, NULL, 0);
	XtAddCallback(command, XtNcallback, monitor ?
		      NewMonitorModeLineCallback : NewModesModeLineCallback,
		      (XtPointer)node);
	node->data->modeline.text =
		XtVaCreateManagedWidget("value", asciiTextWidgetClass, box,
					XtNeditType, XawtextEdit, NULL);
	node->data->modeline.value =
		XtVaCreateManagedWidget("modelineNew", asciiTextWidgetClass, box,
					XtNeditType, XawtextEdit, NULL);
    }
    if (XtIsRealized(node->treeParent))
	XtRealizeWidget(box);
    XtManageChild(box);
}

/*ARGUSED*/
static void
ModesModeLineDestroy(TreeNode *node)
{
    if (node->data->modeline.modeline)
	xf86removeModesModeLine(node->parent->data->modes.modes,
				node->data->modeline.modeline);
}

/*ARGSUSED*/
static void
NewModesModeLineCallback(Widget unused, XtPointer user_data, XtPointer call_data)
{
    TreeNode *parent, *node = (TreeNode*)user_data;
    XF86ConfModeLinePtr modeline;
    Arg args[1];
    char *ident, *value;

    XtSetArg(args[0], XtNstring, &ident);
    XtGetValues(node->data->modeline.text, args, 1);
    XtSetArg(args[0], XtNstring, &value);
    XtGetValues(node->data->modeline.value, args, 1);
    if (*ident == '\0' || *value == '\0')
	return;

    parent = node->parent;
    DeleteNode(node);
    modeline = ParseModeLine(ident, value);
    parent->data->modes.modes->mon_modeline_lst =
	xf86addModeLine(parent->data->modes.modes->mon_modeline_lst, modeline);

    CreateModesModeLine(parent, modeline);
    RelayoutTree();
}

static XF86ConfModeLinePtr
ParseModeLine(char *identifier, char *modeline)
{
    XF86ConfModeLinePtr ml = (XF86ConfModeLinePtr)
	XtCalloc(1, sizeof(XF86ConfModeLineRec));
    char *s, *ptr = modeline;

    /* Identifier */
    ml->ml_identifier = XtNewString(identifier);

    ml->ml_clock = (int)(strtod(ptr, &ptr) * 1000.0 + 0.5);
    while (*ptr && isspace(*ptr)) ++ptr;

    ml->ml_hdisplay = strtol(ptr, &ptr, 10);
    while (*ptr && isspace(*ptr)) ++ptr;

    ml->ml_hsyncstart = strtol(ptr, &ptr, 10);
    while (*ptr && isspace(*ptr)) ++ptr;

    ml->ml_hsyncend = strtol(ptr, &ptr, 10);
    while (*ptr && isspace(*ptr)) ++ptr;

    ml->ml_htotal = strtol(ptr, &ptr, 10);
    while (*ptr && isspace(*ptr)) ++ptr;

    ml->ml_vdisplay = strtol(ptr, &ptr, 10);
    while (*ptr && isspace(*ptr)) ++ptr;

    ml->ml_vsyncstart = strtol(ptr, &ptr, 10);
    while (*ptr && isspace(*ptr)) ++ptr;

    ml->ml_vsyncend = strtol(ptr, &ptr, 10);
    while (*ptr && isspace(*ptr)) ++ptr;

    ml->ml_vtotal = strtol(ptr, &ptr, 10);
    while (*ptr && isspace(*ptr)) ++ptr;

    s = ptr;
    while (*s) {
	*s = tolower(*s);
	++s;
    }
    s = ptr;

    while (*ptr) {
	while (*s && isspace(*s))
	    s++;
	ptr = s;
	while (*s && !isspace(*s))
	    s++;

	if (s != ptr) {
	    Bool done = *s == '\0';

	    *s = '\0';
	    if (strcmp(ptr, "interlace") == 0)
		ml->ml_flags |= XF86CONF_INTERLACE;
	    else if (strcmp(ptr, "+hsync") == 0)
		ml->ml_flags |= XF86CONF_PHSYNC;
	    else if (strcmp(ptr, "-hsync") == 0)
		ml->ml_flags |= XF86CONF_NHSYNC;
	    else if (strcmp(ptr, "+vsync") == 0)
		ml->ml_flags |= XF86CONF_PVSYNC;
	    else if (strcmp(ptr, "-vsync") == 0)
		ml->ml_flags |= XF86CONF_NVSYNC;
	    else if (strcmp(ptr, "composite") == 0)
		ml->ml_flags |= XF86CONF_CSYNC;
	    else if (strcmp(ptr, "+csync") == 0)
		ml->ml_flags |= XF86CONF_PCSYNC;
	    else if (strcmp(ptr, "-csync") == 0)
		ml->ml_flags |= XF86CONF_NCSYNC;
	    else if (strcmp(ptr, "doublescan") == 0)
		ml->ml_flags |= XF86CONF_DBLSCAN;
	    else if (strcmp(ptr, "bcast") == 0)
		ml->ml_flags |= XF86CONF_BCAST;
	    else if (strcmp(ptr, "hskew") == 0) {
		++s;
		while (*s && isspace(*s))
		    ++s;
		ptr = s;
		while (*s && !isspace(*s))
		    ++s;
		if (ptr != s) {
		    ml->ml_hskew = strtol(ptr, &s, 10);
		    ml->ml_flags |= XF86CONF_HSKEW;
		    --s;
		}
	    }
	    else if (strcmp(ptr, "vscan") == 0) {
		++s;
		while (*s && isspace(*s))
		    ++s;
		ptr = s;
		while (*s && !isspace(*s))
		    ++s;
		if (ptr != s) {
		    ml->ml_vscan = strtol(ptr, &s, 10);
		    ml->ml_flags |= XF86CONF_VSCAN;
		    --s;
		}
	    }
	    else if (strcmp(ptr, "custom") == 0)
		ml->ml_flags |= XF86CONF_CUSTOM;
	    ++s;
	    if (done)
		break;
	    ptr = s;
	}
    }

    return (ml);
}

/* VideoAdpator */
static void
CreateVideoAdaptor(TreeNode *parent, XF86ConfVideoAdaptorPtr video)
{
    TreeNode *node, *prev;
    TreeData *data;

    if ((prev = parent->child) != NULL)
	while (prev->next)
	    prev = prev->next;

    while (video) {
	data = (TreeData*)XtCalloc(1, sizeof(TreeData));
	data->video.video = video;
	node = NewNode(parent, NULL, NULL, parent->node, data);
	node->destroy = VideoAdaptorDestroy;
	node->update = VideoAdaptorUpdate;
	CreateVideoAdaptorField(node, False);
	if (parent->child == NULL)
	    parent->child = node;
	else
	    prev->next = node;
	prev = node;

	video = (XF86ConfVideoAdaptorPtr)(video->list.next);
    }

    data = (TreeData*)XtCalloc(1, sizeof(TreeData));
    node = NewNode(parent, NULL, NULL, parent->node, data);
    CreateVideoAdaptorField(node, True);
    if (parent->child == NULL)
	parent->child = node;
    else
	prev->next = node;
}

static void
CreateVideoAdaptorField(TreeNode *node, Bool addnew)
{
    Widget box, command, label;

    box = XtVaCreateWidget("video", formWidgetClass, tree,
			   XtNtreeParent, node->treeParent, NULL);
    node->node = box;

    if (!addnew) {
	char *str;
	TreeNode *port;
	XF86ConfVideoAdaptorPtr video = node->data->video.video;

	command = XtCreateManagedWidget("remove", commandWidgetClass, box,
					NULL, 0);
	XtAddCallback(command, XtNcallback, DestroyCallback, (XtPointer)node);
	command = XtCreateManagedWidget("options", commandWidgetClass, box,
					NULL, 0);
	XtAddCallback(command, XtNcallback, OptionsCallback,
		      (XtPointer)&(video->va_option_lst));
	label = XtVaCreateManagedWidget("adaptor", labelWidgetClass, box,
					XtNlabel, video->va_identifier,
					NULL);

	XtCreateManagedWidget("vendorL", labelWidgetClass, box, NULL, 0);
	str = video->va_vendor ? video->va_vendor : "";
	node->data->video.vendor =
		XtVaCreateManagedWidget("vendor", asciiTextWidgetClass, box,
					XtNeditType, XawtextEdit,
					XtNstring, str,
					NULL);

	XtCreateManagedWidget("boardL", labelWidgetClass, box, NULL, 0);
	str = video->va_board ? video->va_board : "";
	node->data->video.board =
		XtVaCreateManagedWidget("board", asciiTextWidgetClass, box,
					XtNeditType, XawtextEdit,
					XtNstring, str,
					NULL);

	XtCreateManagedWidget("busidL", labelWidgetClass, box, NULL, 0);
	str = video->va_busid ? video->va_busid : "";
	node->data->video.busid =
		XtVaCreateManagedWidget("busid", asciiTextWidgetClass, box,
					XtNeditType, XawtextEdit,
					XtNstring, str,
					NULL);

	XtCreateManagedWidget("driverL", labelWidgetClass, box, NULL, 0);
	str = video->va_driver ? video->va_driver : "";
	node->data->video.driver =
		XtVaCreateManagedWidget("driver", asciiTextWidgetClass, box,
					XtNeditType, XawtextEdit,
					XtNstring, str,
					NULL);

	label = XtVaCreateManagedWidget("VideoPort", toggleWidgetClass, tree,
					XtNstate, True,
					XtNtreeParent, box,
					NULL);
	port = NewNode(node, label, label, node->node, NULL);
	node->child = port;
	CreateVideoPort(port, video->va_port_lst);
    }
    else {
	command = XtCreateManagedWidget("new", commandWidgetClass, box,
					NULL, 0);
	XtAddCallback(command, XtNcallback, NewVideoAdaptorCallback,
		      (XtPointer)node);
	label = XtVaCreateManagedWidget("value", asciiTextWidgetClass, box,
					XtNeditType, XawtextEdit,
					NULL);
	node->data->video.text = label;
    }
    if (XtIsRealized(node->treeParent))
	XtRealizeWidget(box);
    XtManageChild(box);
}

/*ARGUSED*/
static void
VideoAdaptorDestroy(TreeNode *node)
{
    if (node->data->video.video) {
	int i;
	TreeNode *scrn = screenTree->child;

	/* last one is the "new" entry */
	while (scrn && scrn->next) {
	    /* VideoAdator is the first entry */
	    TreeNode *ad = scrn->child->child;
	    CompositeWidget composite;

	    while (ad && ad->next) {
		TreeNode *next = ad->next;

		if (ad && strcmp(ad->data->adaptorlink.adaptorlink->al_adaptor_str,
				  node->data->video.video->va_identifier) == 0)
		    DeleteNode(ad);
		ad = next;
	    }
	    composite = (CompositeWidget)ad->data->adaptorlink.menu;

	    for (i = 0; i < composite->composite.num_children; ++i)
		if (strcmp(XtName(composite->composite.children[i]),
			   node->data->video.video->va_identifier) == 0)
		    XtDestroyWidget(composite->composite.children[i]);

	    scrn = scrn->next;
	}

	xf86removeVideoAdaptor(XF86Config, node->data->video.video);
    }
}

/*ARGSUSED*/
static void
NewVideoAdaptorCallback(Widget unused, XtPointer user_data, XtPointer call_data)
{
    TreeNode *parent, *node = (TreeNode*)user_data;
    XF86ConfVideoAdaptorPtr video;
    Arg args[1];
    char *label;

    XtSetArg(args[0], XtNstring, &label);
    XtGetValues(node->data->video.text, args, 1);
    if (*label == '\0')
	return;

    parent = node->parent;
    DeleteNode(node);
    video = (XF86ConfVideoAdaptorPtr)
	XtCalloc(1, sizeof(XF86ConfVideoAdaptorRec));
    video->va_identifier = XtNewString(label);
    XF86Config->conf_videoadaptor_lst =
	xf86addVideoAdaptor(XF86Config->conf_videoadaptor_lst, video);

    {
	TreeNode *scrn = screenTree->child;
	Widget sme;

	/* last one is the "new" entry */
	while (scrn && scrn->next) {
	    /* VideoAdaptor is the first entry */
	    TreeNode *ad = scrn->child->child;

	    while (ad && ad->next)
		ad = ad->next;

	    sme = XtCreateManagedWidget(video->va_identifier,
					smeBSBObjectClass,
					ad->data->adaptorlink.menu, NULL, 0);
	    XtAddCallback(sme, XtNcallback, NewScreenAdaptorCallback,
			  (XtPointer)ad);

	    scrn = scrn->next;
	}
    }

    CreateVideoAdaptor(parent, video);
    RelayoutTree();
}

static void
VideoAdaptorUpdate(TreeNode *node)
{
    char *str;

    /* vendor */
    XtVaGetValues(node->data->video.vendor, XtNstring, &str, NULL);
    XtFree(node->data->video.video->va_vendor);
    if (*str)
	node->data->video.video->va_vendor = XtNewString(str);
    else
	node->data->video.video->va_vendor = NULL;

    /* board */
    XtVaGetValues(node->data->video.board, XtNstring, &str, NULL);
    XtFree(node->data->video.video->va_board);
    if (*str)
	node->data->video.video->va_board = XtNewString(str);
    else
	node->data->video.video->va_board = NULL;

    /* busid */
    XtVaGetValues(node->data->video.busid, XtNstring, &str, NULL);
    XtFree(node->data->video.video->va_busid);
    if (*str)
	node->data->video.video->va_busid = XtNewString(str);
    else
	node->data->video.video->va_busid = NULL;

    /* driver */
    XtVaGetValues(node->data->video.driver, XtNstring, &str, NULL);
    XtFree(node->data->video.video->va_driver);
    if (*str)
	node->data->video.video->va_driver = XtNewString(str);
    else
	node->data->video.video->va_driver = NULL;
}

static void
CreateVideoPort(TreeNode *parent, XF86ConfVideoPortPtr port)
{
    TreeNode *prev, *node;
    TreeData *data;

    if ((prev = parent->child) != NULL)
	while (prev->next)
	    prev = prev->next;

    while (port) {
	data = (TreeData*)XtCalloc(1, sizeof(TreeData));
	data->port.port = port;
	node = NewNode(parent, NULL, NULL, parent->node, data);
	node->destroy = VideoPortDestroy;
	CreateVideoPortField(node, False);
	if (parent->child == NULL)
	    parent->child = node;
	else
	    prev->next = node;
	prev = node;
	port = (XF86ConfVideoPortPtr)(port->list.next);
    }

    data = (TreeData*)XtCalloc(1, sizeof(TreeData));
    node = NewNode(parent, NULL, NULL, parent->node, data);
    CreateVideoPortField(node, True);
    if (parent->child == NULL)
	parent->child = node;
    else
	prev->next = node;
}

static void
CreateVideoPortField(TreeNode *node, Bool addnew)
{
    Widget box, command, label;

    box = XtVaCreateWidget("port", formWidgetClass, tree,
			   XtNtreeParent, node->treeParent, NULL);
    node->node = box;

    if (!addnew) {
	XF86OptionPtr *options;
	XF86ConfVideoPortPtr port = node->data->port.port;

	options = &(port->vp_option_lst);
	command = XtCreateManagedWidget("remove", commandWidgetClass, box,
					NULL, 0);
	XtAddCallback(command, XtNcallback, DestroyCallback, (XtPointer)node);
	command = XtCreateManagedWidget("options", commandWidgetClass, box,
					NULL, 0);
	XtAddCallback(command, XtNcallback, OptionsCallback, (XtPointer)options);
	label = XtVaCreateManagedWidget("label", labelWidgetClass, box,
					XtNlabel, port->vp_identifier, NULL);
    }
    else {
	command = XtCreateManagedWidget("new", commandWidgetClass, box,
					NULL, 0);
	XtAddCallback(command, XtNcallback, NewVideoPortCallback, (XtPointer)node);
	label = XtVaCreateManagedWidget("value", asciiTextWidgetClass, box,
					XtNeditType, XawtextEdit,
					NULL);
	node->data->port.text = label;
    }
    if (XtIsRealized(node->treeParent))
	XtRealizeWidget(box);
    XtManageChild(box);
}

/*ARGUSED*/
static void
VideoPortDestroy(TreeNode *node)
{
    if (node->data->port.port)
	xf86removeVideoPort(node->parent->parent->data->video.video,
			    node->data->port.port);
}

/*ARGSUSED*/
static void
NewVideoPortCallback(Widget unused, XtPointer user_data, XtPointer call_data)
{
    TreeNode *video, *node = (TreeNode*)user_data;
    XF86ConfVideoPortPtr port;
    Arg args[1];
    char *label;

    XtSetArg(args[0], XtNstring, &label);
    XtGetValues(node->data->port.text, args, 1);
    if (*label == '\0')
	return;

    video = node->parent->parent;
    DeleteNode(node);
    port = (XF86ConfVideoPortPtr)XtCalloc(1, sizeof(XF86ConfVideoPortRec));
    port->vp_identifier = XtNewString(label);
    video->data->video.video->va_port_lst =
	xf86addVideoPort(video->data->video.video->va_port_lst, port);

    CreateVideoPort(video, port);
    RelayoutTree();
}

/* Monitor */
static void
CreateMonitor(TreeNode *parent, XF86ConfMonitorPtr mon)
{
    TreeNode *prev, *node;
    TreeData *data;

    if ((prev = parent->child) != NULL)
	while (prev->next)
	    prev = prev->next;

    while (mon) {
	data = (TreeData*)XtCalloc(1, sizeof(TreeData));
	data->monitor.monitor = mon;
	node = NewNode(parent, NULL, NULL, parent->node, data);
	node->destroy = MonitorDestroy;
	node->update = MonitorUpdate;
	CreateMonitorField(node, False);
	if (parent->child == NULL)
	    parent->child = node;
	else
	    prev->next = node;
	prev = node;
	mon = (XF86ConfMonitorPtr)(mon->list.next);
    }

    data = (TreeData*)XtCalloc(1, sizeof(TreeData));
    node = NewNode(parent, NULL, NULL, parent->node, data);
    CreateMonitorField(node, True);
    if (parent->child == NULL)
	parent->child = node;
    else
	prev->next = node;
}

static void
CreateMonitorField(TreeNode *node, Bool addnew)
{
    Widget box, command, label;

    box = XtVaCreateWidget("monitor", formWidgetClass, tree,
			   XtNtreeParent, node->treeParent, NULL);
    node->node = box;

    if (!addnew) {
	char *str, buf[256];
	XF86OptionPtr *options;
	XF86ConfMonitorPtr mon = node->data->monitor.monitor;
	Widget useModes;
	TreeNode *modeline, *modes, *prev;

	options = &(mon->mon_option_lst);
	command = XtCreateManagedWidget("remove", commandWidgetClass, box,
					NULL, 0);
	XtAddCallback(command, XtNcallback, DestroyCallback, (XtPointer)node);
	command = XtCreateManagedWidget("options", commandWidgetClass, box,
					NULL, 0);
	XtAddCallback(command, XtNcallback, OptionsCallback, (XtPointer)options);
	label = XtVaCreateManagedWidget("label", labelWidgetClass, box,
					XtNlabel, mon->mon_identifier, NULL);

	XtCreateManagedWidget("vendorL", labelWidgetClass, box, NULL, 0);
	str = mon->mon_vendor ? mon->mon_vendor : "";
	node->data->monitor.vendor =
		XtVaCreateManagedWidget("vendor", asciiTextWidgetClass, box,
					XtNeditType, XawtextEdit,
					XtNstring, str,
					NULL);

	XtCreateManagedWidget("modelnameL", labelWidgetClass, box, NULL, 0);
	str = mon->mon_modelname ? mon->mon_modelname : "";
	node->data->monitor.model =
		XtVaCreateManagedWidget("modelname", asciiTextWidgetClass, box,
					XtNeditType, XawtextEdit,
					XtNstring, str,
					NULL);

	XtCreateManagedWidget("widthL", labelWidgetClass, box, NULL, 0);
	if (mon->mon_width)
	    XmuSnprintf(buf, sizeof(buf), "%d", mon->mon_width);
	else
	    *buf = '\0';
	node->data->monitor.width =
		XtVaCreateManagedWidget("width", asciiTextWidgetClass, box,
					XtNeditType, XawtextEdit,
					XtNstring, buf,
					NULL);

	XtCreateManagedWidget("heightL", labelWidgetClass, box, NULL, 0);
	if (mon->mon_height)
	    XmuSnprintf(buf, sizeof(buf), "%d", mon->mon_height);
	else
	    *buf = '\0';
	node->data->monitor.height =
		XtVaCreateManagedWidget("height", asciiTextWidgetClass, box,
					XtNeditType, XawtextEdit,
					XtNstring, buf,
					NULL);

	XtCreateManagedWidget("hsyncL", labelWidgetClass, box, NULL, 0);
	if (mon->mon_n_hsync > 0)
	    parser_range_to_string(buf, &(mon->mon_hsync[0]),
				   mon->mon_n_hsync);
	else
	    *buf = '\0';
	node->data->monitor.hsync =
		XtVaCreateManagedWidget("hsync", asciiTextWidgetClass, box,
					XtNeditType, XawtextEdit,
					XtNstring, buf,
					NULL);

	XtCreateManagedWidget("vrefreshL", labelWidgetClass, box, NULL, 0);
	if (mon->mon_n_vrefresh > 0)
	    parser_range_to_string(buf, &(mon->mon_vrefresh[0]),
				   mon->mon_n_vrefresh);
	else
	    *buf = '\0';
	node->data->monitor.vrefresh =
		XtVaCreateManagedWidget("vrefresh", asciiTextWidgetClass, box,
					XtNeditType, XawtextEdit,
					XtNstring, buf,
					NULL);

	XtCreateManagedWidget("gammaRedL", labelWidgetClass, box, NULL, 0);
	if (mon->mon_gamma_red)	
	    XmuSnprintf(buf, sizeof(buf), "%g", mon->mon_gamma_red);
	else
	    *buf = '\0';
	node->data->monitor.gammaRed =
		XtVaCreateManagedWidget("gammaRed", asciiTextWidgetClass, box,
					XtNeditType, XawtextEdit,
					XtNstring, buf,
					NULL);

	XtCreateManagedWidget("gammaGreenL", labelWidgetClass, box, NULL, 0);
	if (mon->mon_gamma_green)
	    XmuSnprintf(buf, sizeof(buf), "%g", mon->mon_gamma_green);
	else
	    *buf = '\0';
	node->data->monitor.gammaGreen =
		XtVaCreateManagedWidget("gammaGreen", asciiTextWidgetClass, box,
					XtNeditType, XawtextEdit,
					XtNstring, buf,
					NULL);

	XtCreateManagedWidget("gammaBlueL", labelWidgetClass, box, NULL, 0);
	if (mon->mon_gamma_blue)
	    XmuSnprintf(buf, sizeof(buf), "%g", mon->mon_gamma_blue);
	else
	    *buf = '\0';
	node->data->monitor.gammaBlue =
		XtVaCreateManagedWidget("gammaBlue", asciiTextWidgetClass, box,
					XtNeditType, XawtextEdit,
					XtNstring, buf,
					NULL);

	if ((prev = node->child) != NULL)
	    while (prev->next)
		prev = prev->next;
	command = XtVaCreateManagedWidget("ModeLine", toggleWidgetClass, tree,
					  XtNstate, True,
					  XtNtreeParent, box, NULL);
	modeline = NewNode(node, command, command, node->node, NULL);
	CreateMonitorModeLine(modeline,
			      node->data->monitor.monitor->mon_modeline_lst);
	if (prev == NULL)
	    prev = node->child = modeline;
	else {
	    prev->next = modeline;
	    prev = prev->next;
	}

	useModes = XtVaCreateManagedWidget("UseModes", toggleWidgetClass, tree,
					   XtNstate, True,
					   XtNtreeParent, box, NULL);
	prev->next = modes = NewNode(node, useModes, useModes, node->node, NULL);
	CreateMonitorModes(modes,
			   node->data->monitor.monitor->mon_modes_sect_lst);
    }
    else {
	command = XtCreateManagedWidget("new", commandWidgetClass, box,
					NULL, 0);
	XtAddCallback(command, XtNcallback, NewMonitorCallback, (XtPointer)node);
	label = XtVaCreateManagedWidget("value", asciiTextWidgetClass, box,
					XtNeditType, XawtextEdit,
					NULL);
	node->data->monitor.text = label;
    }
    if (XtIsRealized(node->treeParent))
	XtRealizeWidget(box);
    XtManageChild(box);
}

static void
MonitorDestroy(TreeNode *node)
{
    int i;
    TreeNode *sc = screenTree;

    for (i = 0; i < computer.num_devices; i++)
	if ((XF86ConfMonitorPtr)(computer.devices[i]->config) ==
	    node->data->monitor.monitor) {
	    config = computer.devices[i]->widget;
	    RemoveDeviceCallback(NULL, NULL, NULL);
	}

    if (sc) {
	TreeNode *prev;

	sc = prev = sc->child;
	while (sc->next) {
	    TreeNode *next = sc->next;

	    if (sc->data->screen.screen->scrn_monitor ==
		node->data->monitor.monitor) {
		XtDestroyWidget(sc->node);

		if (sc->child)
		    DestroyTree(sc->child);
		if (sc->data)
		    XtFree((XtPointer)sc->data);
		XtFree((XtPointer)sc);

		if (sc == screenTree->child)
		    sc = prev = next = screenTree->child = next;
		else
		    prev->next = sc = next;
		continue;
	    }
	    prev = sc;
	    sc = next;
	}
    }
}

static void
NewMonitorCallback(Widget w, XtPointer user_data, XtPointer call_data)
{
    TreeNode *parent, *node = (TreeNode*)user_data;
    XF86ConfMonitorPtr mon;
    Arg args[1];
    char *label;

    XtSetArg(args[0], XtNstring, &label);
    XtGetValues(node->data->monitor.text, args, 1);
    if (*label == '\0')
	return;

    parent = node->parent;
    DeleteNode(node);
    mon = (XF86ConfMonitorPtr)XtCalloc(1, sizeof(XF86ConfMonitorRec));
    mon->mon_identifier = XtNewString(label);
    XF86Config->conf_monitor_lst =
	xf86addMonitor(XF86Config->conf_monitor_lst, mon);

    CreateMonitor(parent, mon);

    RelayoutTree();
}

static void
MonitorUpdate(TreeNode *node)
{
    char *str;

    /* vendor */
    XtVaGetValues(node->data->monitor.vendor, XtNstring, &str, NULL);
    XtFree(node->data->monitor.monitor->mon_vendor);
    if (*str)
	node->data->monitor.monitor->mon_vendor = XtNewString(str);
    else
	node->data->monitor.monitor->mon_vendor = NULL;

    /* model */
    XtVaGetValues(node->data->monitor.model, XtNstring, &str, NULL);
    XtFree(node->data->monitor.monitor->mon_modelname);
    if (*str)
	node->data->monitor.monitor->mon_modelname = XtNewString(str);
    else
	node->data->monitor.monitor->mon_modelname = NULL;

    /* width */
    XtVaGetValues(node->data->monitor.width, XtNstring, &str, NULL);
    node->data->monitor.monitor->mon_width = strtoul(str, NULL, 0);

    /* height */
    XtVaGetValues(node->data->monitor.height, XtNstring, &str, NULL);
    node->data->monitor.monitor->mon_height = strtoul(str, NULL, 0);

    /* hsync */
    XtVaGetValues(node->data->monitor.hsync, XtNstring, &str, NULL);
    node->data->monitor.monitor->mon_n_hsync =
	string_to_parser_range(str,
			       &(node->data->monitor.monitor->mon_hsync[0]),
			       CONF_MAX_HSYNC);

    /* vrefresh */
    XtVaGetValues(node->data->monitor.vrefresh, XtNstring, &str, NULL);
    node->data->monitor.monitor->mon_n_vrefresh =
	string_to_parser_range(str,
			       &(node->data->monitor.monitor->mon_vrefresh[0]),
			       CONF_MAX_VREFRESH);

    /* gammaRed */
    XtVaGetValues(node->data->monitor.gammaRed, XtNstring, &str, NULL);
    node->data->monitor.monitor->mon_gamma_red = strtod(str, NULL);

    /* gammaGreen */
    XtVaGetValues(node->data->monitor.gammaGreen, XtNstring, &str, NULL);
    node->data->monitor.monitor->mon_gamma_green = strtod(str, NULL);

    /* gammaBlue */
    XtVaGetValues(node->data->monitor.gammaBlue, XtNstring, &str, NULL);
    node->data->monitor.monitor->mon_gamma_blue = strtod(str, NULL);
}

static void
CreateMonitorModeLine(TreeNode *parent, XF86ConfModeLinePtr modeline)
{
    TreeNode *node, *prev;
    TreeData *data;

    if ((prev = parent->child) != NULL)
	while (prev->next)
	    prev = prev->next;

    while (modeline) {
	data = (TreeData*)XtCalloc(1, sizeof(TreeData));
	data->modeline.modeline = modeline;
	node = NewNode(parent, NULL, NULL, parent->node, data);
	node->destroy = MonitorModeLineDestroy;
	CreateModeLineField(node, False, True);
	if (parent->child == NULL)
	    parent->child = node;
	else
	    prev->next = node;
	prev = node;
	modeline = (XF86ConfModeLinePtr)(modeline->list.next);
    }
    data = (TreeData*)XtCalloc(1, sizeof(TreeData));
    node = NewNode(parent, NULL, NULL, parent->node, data);
    if (parent->child == NULL)
	parent->child = node;
    else
	prev->next = node;
    prev = node;
    CreateModeLineField(node, True, True);
}

/*ARGUSED*/
static void
MonitorModeLineDestroy(TreeNode *node)
{
    if (node->data->modeline.modeline)
	xf86removeMonitorModeLine(node->parent->parent->data->monitor.monitor,
				  node->data->modeline.modeline);
}

/*ARGSUSED*/
static void
NewMonitorModeLineCallback(Widget w, XtPointer user_data, XtPointer call_data)
{
    TreeNode *parent, *node = (TreeNode*)user_data;
    XF86ConfModeLinePtr modeline;
    Arg args[1];
    char *ident, *value;

    XtSetArg(args[0], XtNstring, &ident);
    XtGetValues(node->data->modeline.text, args, 1);
    XtSetArg(args[0], XtNstring, &value);
    XtGetValues(node->data->modeline.value, args, 1);
    if (*ident == '\0' || *value == '\0')
	return;

    parent = node->parent;
    DeleteNode(node);
    modeline = ParseModeLine(ident, value);
    parent->parent->data->monitor.monitor->mon_modeline_lst =
	xf86addModeLine(parent->parent->data->monitor.monitor->mon_modeline_lst,
			modeline);

    CreateMonitorModeLine(parent, modeline);
    RelayoutTree();
}

static void
CreateMonitorModes(TreeNode *parent, XF86ConfModesLinkPtr lnk)
{
    TreeNode *node, *prev;
    TreeData *data;

    if ((prev = parent->child) != NULL)
	while (prev->next)
	    prev = prev->next;

    while (lnk) {
	data = (TreeData*)XtCalloc(1, sizeof(TreeData));
	data->modeslink.modeslink = lnk;
	node = NewNode(parent, NULL, NULL, parent->node, data);
	node->destroy = MonitorModesLinkDestroy;
	CreateMonitorModesField(node, False);
	if (parent->child == NULL)
	    parent->child = node;
	else
	    prev->next = node;
	prev = node;
	lnk = (XF86ConfModesLinkPtr)(lnk->list.next);
    }
    data = (TreeData*)XtCalloc(1, sizeof(TreeData));
    node = NewNode(parent, NULL, NULL, parent->node, data);
    if (parent->child == NULL)
	parent->child = node;
    else
	prev->next = node;
    prev = node;
    CreateMonitorModesField(node, True);
}

static void
CreateMonitorModesField(TreeNode *node, Bool addnew)
{
    Widget box, command;

    box = XtVaCreateWidget("modes", formWidgetClass, tree,
			   XtNtreeParent, node->treeParent, NULL);
    node->node = box;

    if (!addnew) {
	XF86ConfModesLinkPtr lnk = node->data->modeslink.modeslink;

	command = XtCreateManagedWidget("remove", commandWidgetClass, box,
					NULL, 0);
	XtAddCallback(command, XtNcallback, DestroyCallback, (XtPointer)node);
	(void) XtVaCreateManagedWidget("mode", labelWidgetClass, box,
					XtNlabel, lnk->ml_modes_str, NULL);
    }
    else {
	Widget sme;
	XF86ConfModesPtr ptr = XF86Config->conf_modes_lst;

	command = XtVaCreateManagedWidget("new", menuButtonWidgetClass, box,
					  XtNmenuName, "modesMenu", NULL);
	node->data->modeslink.menu =
	    XtVaCreatePopupShell("modesMenu", simpleMenuWidgetClass, box,
				 XtNleftMargin, 1, XtNrightMargin, 1,
				 XtNtopMargin, 1, XtNbottomMargin, 1,
				 NULL);
	while (ptr) {
	    sme = XtCreateManagedWidget(ptr->modes_identifier, smeBSBObjectClass,
					node->data->modeslink.menu, NULL, 0);
	    XtAddCallback(sme, XtNcallback, NewMonitorModesCallback,
			  (XtPointer)node);
	    ptr = (XF86ConfModesPtr)(ptr->list.next);
	}
    }
    if (XtIsRealized(node->treeParent))
	XtRealizeWidget(box);
    XtManageChild(box);
}

/*ARGUSED*/
static void
MonitorModesLinkDestroy(TreeNode *node)
{
    if (node->data->modeslink.modeslink)
	xf86removeMonitorModesLink(node->parent->parent->data->monitor.monitor,
				   node->data->modeslink.modeslink);
}

/*ARGSUSED*/
static void
NewMonitorModesCallback(Widget w, XtPointer user_data, XtPointer call_data)
{
    TreeNode *parent, *node = (TreeNode*)user_data;
    XF86ConfModesLinkPtr link;
    char *ident = XtName(w);

    parent = node->parent;
    DeleteNode(node);
    link = (XF86ConfModesLinkPtr)XtCalloc(1, sizeof(XF86ConfModesLinkRec));
    link->ml_modes_str = XtNewString(ident);
    parent->parent->data->monitor.monitor->mon_modes_sect_lst =
	xf86addModesLink(parent->parent->data->monitor.monitor->mon_modes_sect_lst,
			link);

    CreateMonitorModes(parent, link);
    RelayoutTree();
}

/* Device */
static void
CreateDevice(TreeNode *parent, XF86ConfDevicePtr dev)
{
    TreeNode *prev, *node;
    TreeData *data;

    if ((prev = parent->child) != NULL)
	while (prev->next)
	    prev = prev->next;

    while (dev) {
	data = (TreeData*)XtCalloc(1, sizeof(TreeData));
	data->device.device = dev;
	node = NewNode(parent, NULL, NULL, parent->node, data);
	node->destroy = DeviceDestroy;
	node->update = DeviceUpdate;
	CreateDeviceField(node, False);
	if (parent->child == NULL)
	    parent->child = node;
	else
	    prev->next = node;
	prev = node;
	dev = (XF86ConfDevicePtr)(dev->list.next);
    }

    data = (TreeData*)XtCalloc(1, sizeof(TreeData));
    node = NewNode(parent, NULL, NULL, parent->node, data);
    CreateDeviceField(node, True);
    if (parent->child == NULL)
	parent->child = node;
    else
	prev->next = node;
}

static void
CreateDeviceField(TreeNode *node, Bool addnew)
{
    Widget box, command, label;

    box = XtVaCreateWidget("device", formWidgetClass, tree,
			   XtNtreeParent, node->treeParent, NULL);
    node->node = box;

    if (!addnew) {
	int i, tmp, len;
	char buf[1024], *str;
	XF86OptionPtr *options;
	XF86ConfDevicePtr dev = node->data->device.device;

	options = &(dev->dev_option_lst);
	command = XtCreateManagedWidget("remove", commandWidgetClass, box,
					NULL, 0);
	XtAddCallback(command, XtNcallback, DestroyCallback, (XtPointer)node);
	command = XtCreateManagedWidget("options", commandWidgetClass, box,
					NULL, 0);
	XtAddCallback(command, XtNcallback, OptionsCallback, (XtPointer)options);
	label = XtVaCreateManagedWidget("label", labelWidgetClass, box,
					XtNlabel, dev->dev_identifier, NULL);

	XtCreateManagedWidget("vendorL", labelWidgetClass, box, NULL, 0);
	str = dev->dev_vendor ? dev->dev_vendor : "";
	node->data->device.vendor =
		XtVaCreateManagedWidget("vendor", asciiTextWidgetClass, box,
					XtNeditType, XawtextEdit,
					XtNstring, str,
					NULL);

	XtCreateManagedWidget("boardL", labelWidgetClass, box, NULL, 0);
	str = dev->dev_board ? dev->dev_board : "";
	node->data->device.board =
		XtVaCreateManagedWidget("board", asciiTextWidgetClass, box,
					XtNeditType, XawtextEdit,
					XtNstring, str,
					NULL);

	XtCreateManagedWidget("chipsetL", labelWidgetClass, box, NULL, 0);
	str = dev->dev_chipset ? dev->dev_chipset : "";
	node->data->device.chipset =
		XtVaCreateManagedWidget("chipset", asciiTextWidgetClass, box,
					XtNeditType, XawtextEdit,
					XtNstring, str,
					NULL);

	XtCreateManagedWidget("busidL", labelWidgetClass, box, NULL, 0);
	str = dev->dev_busid ? dev->dev_busid : "";
	node->data->device.busid =
		XtVaCreateManagedWidget("busid", asciiTextWidgetClass, box,
					XtNeditType, XawtextEdit,
					XtNstring, str,
					NULL);

	XtCreateManagedWidget("cardL", labelWidgetClass, box, NULL, 0);
	str = dev->dev_card ? dev->dev_card : "";
	node->data->device.card =
		XtVaCreateManagedWidget("card", asciiTextWidgetClass, box,
					XtNeditType, XawtextEdit,
					XtNstring, str,
					NULL);

	XtCreateManagedWidget("driverL", labelWidgetClass, box, NULL, 0);
	str = dev->dev_driver ? dev->dev_driver : "";
	node->data->device.driver =
		XtVaCreateManagedWidget("driver", asciiTextWidgetClass, box,
					XtNeditType, XawtextEdit,
					XtNstring, str,
					NULL);

	XtCreateManagedWidget("ramdacL", labelWidgetClass, box, NULL, 0);
	str = dev->dev_ramdac ? dev->dev_ramdac : "";
	node->data->device.ramdac =
		XtVaCreateManagedWidget("ramdac", asciiTextWidgetClass, box,
					XtNeditType, XawtextEdit,
					XtNstring, str,
					NULL);

	XtCreateManagedWidget("dacSpeedL", labelWidgetClass, box, NULL, 0);
	if (dev->dev_dacSpeeds[0] > 0) {
	    for (i = len = 0; i < CONF_MAXDACSPEEDS &&
			      dev->dev_dacSpeeds[i] > 0; i++) {
		tmp = XmuSnprintf(buf + len, sizeof(buf) - len, "%g ",
				  dev->dev_dacSpeeds[i] / 1000.);
		len += tmp;
	    }
	}
	else
	    *buf = '\0';
	node->data->device.dacSpeed =
		XtVaCreateManagedWidget("dacSpeed", asciiTextWidgetClass, box,
					XtNeditType, XawtextEdit,
					XtNstring, buf,
					NULL);

	XtCreateManagedWidget("videoRamL", labelWidgetClass, box, NULL, 0);
	if (dev->dev_videoram)
	    XmuSnprintf(buf, sizeof(buf), "%d", dev->dev_videoram);
	else
	    *buf = '\0';
	node->data->device.videoRam =
		XtVaCreateManagedWidget("videoRam", asciiTextWidgetClass, box,
					XtNeditType, XawtextEdit,
					XtNstring, buf,
					NULL);

	XtCreateManagedWidget("textClockFreqL", labelWidgetClass, box, NULL, 0);
	if (dev->dev_textclockfreq)
	    XmuSnprintf(buf, sizeof(buf), "%.1f",
			(double)dev->dev_textclockfreq / 1000.0);
	else
	    *buf = '\0';
	node->data->device.textClockFreq =
		XtVaCreateManagedWidget("textClockFreq", asciiTextWidgetClass, box,
					XtNeditType, XawtextEdit,
					XtNstring, buf,
					NULL);

	XtCreateManagedWidget("biosBaseL", labelWidgetClass, box, NULL, 0);
	if (dev->dev_bios_base)
	    XmuSnprintf(buf, sizeof(buf), "0x%lx", dev->dev_bios_base);
	else
	    *buf = '\0';
	node->data->device.biosBase =
		XtVaCreateManagedWidget("biosBase", asciiTextWidgetClass, box,
					XtNeditType, XawtextEdit,
					XtNstring, buf,
					NULL);

	XtCreateManagedWidget("memBaseL", labelWidgetClass, box, NULL, 0);
	if (dev->dev_mem_base)
	    XmuSnprintf(buf, sizeof(buf), "0x%lx", dev->dev_mem_base);
	else
	    *buf = '\0';
	node->data->device.memBase =
		XtVaCreateManagedWidget("memBase", asciiTextWidgetClass, box,
					XtNeditType, XawtextEdit,
					XtNstring, buf,
					NULL);

	XtCreateManagedWidget("ioBaseL", labelWidgetClass, box, NULL, 0);
	if (dev->dev_io_base)
	    XmuSnprintf(buf, sizeof(buf), "0x%lx", dev->dev_io_base);
	else
	    *buf = '\0';
	node->data->device.ioBase =
		XtVaCreateManagedWidget("ioBase", asciiTextWidgetClass, box,
					XtNeditType, XawtextEdit,
					XtNstring, buf,
					NULL);

	XtCreateManagedWidget("clockChipL", labelWidgetClass, box, NULL, 0);
	str = dev->dev_clockchip ? dev->dev_clockchip : "";
	node->data->device.clockChip =
		XtVaCreateManagedWidget("clockChip", asciiTextWidgetClass, box,
					XtNeditType, XawtextEdit,
					XtNstring, str,
					NULL);

	*buf = '\0';
	for (i = len = 0; i < dev->dev_clocks; i++) {
	    tmp = XmuSnprintf(buf + len, sizeof(buf) - len, "%.1f ",
			      (double)dev->dev_clock[i] / 1000.0);
	    len += tmp;
	}
	XtCreateManagedWidget("devClockL", labelWidgetClass, box, NULL, 0);
	node->data->device.devClock =
		XtVaCreateManagedWidget("devClock", asciiTextWidgetClass, box,
					XtNeditType, XawtextEdit,
					XtNstring, buf,
					NULL);

	XtCreateManagedWidget("chipIdL", labelWidgetClass, box, NULL, 0);
	if (dev->dev_chipid != -1)
	    XmuSnprintf(buf, sizeof(buf), "0x%x", dev->dev_chipid);
	else
	    *buf = '\0';
	node->data->device.chipId =
		XtVaCreateManagedWidget("chipId", asciiTextWidgetClass, box,
					XtNeditType, XawtextEdit,
					XtNstring, buf,
					NULL);

	XtCreateManagedWidget("chipRevL", labelWidgetClass, box, NULL, 0);
	if (dev->dev_chiprev != -1)
	    XmuSnprintf(buf, sizeof(buf), "0x%x", dev->dev_chiprev);
	else
	    *buf = '\0';
	node->data->device.chipRev =
		XtVaCreateManagedWidget("chipRev", asciiTextWidgetClass, box,
					XtNeditType, XawtextEdit,
					XtNstring, buf,
					NULL);

	XtCreateManagedWidget("irqL", labelWidgetClass, box, NULL, 0);
	if (dev->dev_irq != -1)
	    XmuSnprintf(buf, sizeof(buf), "%d", dev->dev_irq);
	else
	    *buf = '\0';
	node->data->device.irq =
		XtVaCreateManagedWidget("irq", asciiTextWidgetClass, box,
					XtNeditType, XawtextEdit,
					XtNstring, buf,
					NULL);

	XtCreateManagedWidget("screenL", labelWidgetClass, box, NULL, 0);
	if (dev->dev_screen > 0)
	    XmuSnprintf(buf, sizeof(buf), "%d", dev->dev_screen);
	else
	    *buf = '\0';
	node->data->device.screen =
		XtVaCreateManagedWidget("screen", asciiTextWidgetClass, box,
					XtNeditType, XawtextEdit,
					XtNstring, buf,
					NULL);
    }
    else {
	command = XtCreateManagedWidget("new", commandWidgetClass, box,
					NULL, 0);
	XtAddCallback(command, XtNcallback, NewDeviceCallback, (XtPointer)node);
	label = XtVaCreateManagedWidget("value", asciiTextWidgetClass, box,
					XtNeditType, XawtextEdit,
					NULL);
	node->data->device.text = label;
    }
    if (XtIsRealized(node->treeParent))
	XtRealizeWidget(box);
    XtManageChild(box);
}

static void
NewDeviceCallback(Widget w, XtPointer user_data, XtPointer call_data)
{
    TreeNode *parent, *node = (TreeNode*)user_data;
    XF86ConfDevicePtr dev;
    Arg args[1];
    char *label;

    XtSetArg(args[0], XtNstring, &label);
    XtGetValues(node->data->device.text, args, 1);
    if (*label == '\0')
	return;

    parent = node->parent;
    DeleteNode(node);
    dev = (XF86ConfDevicePtr)XtCalloc(1, sizeof(XF86ConfDeviceRec));
    dev->dev_identifier = XtNewString(label);
    dev->dev_chipid = -1;
    dev->dev_chiprev = -1;
    dev->dev_irq = -1;

    XF86Config->conf_device_lst =
	xf86addDevice(XF86Config->conf_device_lst, dev);

    CreateDevice(parent, dev);

    RelayoutTree();
}

static void
DeviceDestroy(TreeNode *node)
{
    int i;
    TreeNode *sc = screenTree;

    for (i = 0; i < computer.num_devices; i++)
	if ((XF86ConfDevicePtr)(computer.devices[i]->config) ==
	    node->data->device.device) {
	    config = computer.devices[i]->widget;
	    RemoveDeviceCallback(NULL, NULL, NULL);
	}

    if (sc) {
	TreeNode *prev;

	sc = prev = sc->child;
	while (sc->next) {
	    TreeNode *next = sc->next;

	    if (sc->data->screen.screen->scrn_monitor ==
		node->data->monitor.monitor) {
		XtDestroyWidget(sc->node);

		if (sc->child)
		    DestroyTree(sc->child);
		if (sc->data)
		    XtFree((XtPointer)sc->data);
		XtFree((XtPointer)sc);

		if (sc == screenTree->child)
		    sc = prev = next = screenTree->child = next;
		else
		    prev->next = sc = next;
		continue;
	    }
	    prev = sc;
	    sc = next;
	}
    }
}

static void
DeviceUpdate(TreeNode *node)
{
    int i;
    char *str, *tmp;

    /* vendor */
    XtVaGetValues(node->data->device.vendor, XtNstring, &str, NULL);
    XtFree(node->data->device.device->dev_vendor);
    if (*str)
	node->data->device.device->dev_vendor = XtNewString(str);
    else
	node->data->device.device->dev_vendor = NULL;

    /* board */
    XtVaGetValues(node->data->device.board, XtNstring, &str, NULL);
    XtFree(node->data->device.device->dev_board);
    if (*str)
	node->data->device.device->dev_board = XtNewString(str);
    else
	node->data->device.device->dev_board = NULL;

    /* chipset */
    XtVaGetValues(node->data->device.chipset, XtNstring, &str, NULL);
    XtFree(node->data->device.device->dev_chipset);
    if (*str)
	node->data->device.device->dev_chipset = XtNewString(str);
    else
	node->data->device.device->dev_chipset = NULL;

    /* busid */
    XtVaGetValues(node->data->device.busid, XtNstring, &str, NULL);
    XtFree(node->data->device.device->dev_busid);
    if (*str)
	node->data->device.device->dev_busid = XtNewString(str);
    else
	node->data->device.device->dev_busid = NULL;

    /* card */
    XtVaGetValues(node->data->device.card, XtNstring, &str, NULL);
    XtFree(node->data->device.device->dev_card);
    if (*str)
	node->data->device.device->dev_card = XtNewString(str);
    else
	node->data->device.device->dev_card = NULL;

    /* driver */
    XtVaGetValues(node->data->device.driver, XtNstring, &str, NULL);
    XtFree(node->data->device.device->dev_driver);
    if (*str)
	node->data->device.device->dev_driver = XtNewString(str);
    else
	node->data->device.device->dev_driver = NULL;

    /* ramdac */
    XtVaGetValues(node->data->device.ramdac, XtNstring, &str, NULL);
    XtFree(node->data->device.device->dev_ramdac);
    if (*str)
	node->data->device.device->dev_ramdac = XtNewString(str);
    else
	node->data->device.device->dev_ramdac = NULL;

    /* dacSpeed */
    tmp = NULL;
    XtVaGetValues(node->data->device.dacSpeed, XtNstring, &str, NULL);
    for (i = 0; i < CONF_MAXDACSPEEDS && str != tmp; i++) {
	if ((node->data->device.device->dev_dacSpeeds[i] =
	    (strtod(str, &tmp) * 1000. + .5)) == 0)
	    break;
	str = tmp;
	while (isspace(*str))
	    ++str;
    }

    /* videoRam */
    XtVaGetValues(node->data->device.videoRam, XtNstring, &str, NULL);
    node->data->device.device->dev_videoram = strtoul(str, NULL, 0);

    /* textClockFreq */
    XtVaGetValues(node->data->device.textClockFreq, XtNstring, &str, NULL);
    node->data->device.device->dev_textclockfreq =
	strtod(str, NULL) * 1000. + .5;

    /* biosBase */
    XtVaGetValues(node->data->device.biosBase, XtNstring, &str, NULL);
    node->data->device.device->dev_bios_base = strtoul(str, NULL, 0);

    /* memBase */
    XtVaGetValues(node->data->device.memBase, XtNstring, &str, NULL);
    node->data->device.device->dev_mem_base = strtoul(str, NULL, 0);

    /* ioBase */
    XtVaGetValues(node->data->device.ioBase, XtNstring, &str, NULL);
    node->data->device.device->dev_io_base = strtoul(str, NULL, 0);

    /* clockChip */
    XtVaGetValues(node->data->device.clockChip, XtNstring, &str, NULL);
    XtFree(node->data->device.device->dev_clockchip);
    if (*str)
	node->data->device.device->dev_clockchip = XtNewString(str);
    else
	node->data->device.device->dev_clockchip = NULL;

    /* devSpeed */
    tmp = NULL;
    XtVaGetValues(node->data->device.devClock, XtNstring, &str, NULL);
    for (i = 0; i < CONF_MAXCLOCKS && str != tmp; i++) {
	if ((node->data->device.device->dev_clock[i] =
	     (strtod(str, &tmp) * 1000. + .5)) == 0)
	    break;
	str = tmp;
	while (isspace(*str))
	    ++str;
    }
    node->data->device.device->dev_clocks = i;

    /* chipId */
    XtVaGetValues(node->data->device.chipId, XtNstring, &str, NULL);
    if (*str)
	node->data->device.device->dev_chipid = strtoul(str, NULL, 0);
    else
	node->data->device.device->dev_chipid = -1;

    /* chipRev */
    XtVaGetValues(node->data->device.chipRev, XtNstring, &str, NULL);
    if (*str)
	node->data->device.device->dev_chiprev = strtoul(str, NULL, 0);
    else
	node->data->device.device->dev_chiprev = -1;

    /* irq */
    XtVaGetValues(node->data->device.irq, XtNstring, &str, NULL);
    if (*str)
	node->data->device.device->dev_irq = strtoul(str, NULL, 0);
    else
	node->data->device.device->dev_irq = -1;

    /* screen */
    XtVaGetValues(node->data->device.screen, XtNstring, &str, NULL);
    if (*str)
	node->data->device.device->dev_screen = strtoul(str, NULL, 0);
    else
	node->data->device.device->dev_screen = -1;
}

/* Screen */
static void
CreateScreen(TreeNode *parent, XF86ConfScreenPtr scrn)
{
    TreeNode *prev, *node;
    TreeData *data;

    if ((prev = parent->child) != NULL)
	while (prev->next)
	    prev = prev->next;

    while (scrn) {
	data = (TreeData*)XtCalloc(1, sizeof(TreeData));
	data->screen.screen = scrn;
	node = NewNode(parent, NULL, NULL, parent->node, data);
	node->destroy = ScreenDestroy;
	node->update = ScreenUpdate;
	CreateScreenField(node, False);
	if (parent->child == NULL)
	    parent->child = node;
	else
	    prev->next = node;
	prev = node;
	scrn = (XF86ConfScreenPtr)(scrn->list.next);
    }

    data = (TreeData*)XtCalloc(1, sizeof(TreeData));
    node = NewNode(parent, NULL, NULL, parent->node, data);
    CreateScreenField(node, True);
    if (parent->child == NULL)
	parent->child = node;
    else
	prev->next = node;
}

static void
CreateScreenField(TreeNode *node, Bool addnew)
{
    Widget box, command, label;

    box = XtVaCreateWidget("screen", formWidgetClass, tree,
			   XtNtreeParent, node->treeParent, NULL);
    node->node = box;

    if (!addnew) {
	char buf[256], *str;
	XF86OptionPtr *options;
	TreeNode *adaptor, *display;
	XF86ConfScreenPtr scrn = node->data->screen.screen;

	options = &(scrn->scrn_option_lst);
	command = XtCreateManagedWidget("remove", commandWidgetClass, box,
					NULL, 0);
	XtAddCallback(command, XtNcallback, DestroyCallback, (XtPointer)node);
	command = XtCreateManagedWidget("options", commandWidgetClass, box,
					NULL, 0);
	XtAddCallback(command, XtNcallback, OptionsCallback, (XtPointer)options);
	label = XtVaCreateManagedWidget("label", labelWidgetClass, box,
					XtNlabel, scrn->scrn_identifier, NULL);

	XtCreateManagedWidget("defaultDepthL", labelWidgetClass, box, NULL, 0);
	if (scrn->scrn_defaultdepth)
	    XmuSnprintf(buf, sizeof(buf), "%d", scrn->scrn_defaultdepth);
	else
	    *buf = '\0';
	node->data->screen.defaultDepth =
		XtVaCreateManagedWidget("defaultDepth", asciiTextWidgetClass, box,
					XtNeditType, XawtextEdit,
					XtNstring, buf,
					NULL);

	XtCreateManagedWidget("defaultBppL", labelWidgetClass, box, NULL, 0);
	if (scrn->scrn_defaultbpp)
	    XmuSnprintf(buf, sizeof(buf), "%d", scrn->scrn_defaultbpp);
	else
	    *buf = '\0';
	node->data->screen.defaultBpp =
		XtVaCreateManagedWidget("defaultBpp", asciiTextWidgetClass, box,
					XtNeditType, XawtextEdit,
					XtNstring, buf,
					NULL);

	XtCreateManagedWidget("defaultFbBppL", labelWidgetClass, box, NULL, 0);
	if (scrn->scrn_defaultfbbpp)
	    XmuSnprintf(buf, sizeof(buf), "%d", scrn->scrn_defaultfbbpp);
	else
	    *buf = '\0';
	node->data->screen.defaultFbBpp =
		XtVaCreateManagedWidget("defaultFbBpp", asciiTextWidgetClass, box,
					XtNeditType, XawtextEdit,
					XtNstring, buf,
					NULL);

	XtCreateManagedWidget("monitorL", labelWidgetClass, box, NULL, 0);
	str = scrn->scrn_monitor_str ? scrn->scrn_monitor_str : "";
	node->data->screen.monitor =
		XtVaCreateManagedWidget("monitor", asciiTextWidgetClass, box,
					XtNeditType, XawtextEdit,
					XtNstring, str,
					NULL);

	XtCreateManagedWidget("deviceL", labelWidgetClass, box, NULL, 0);
	str = scrn->scrn_device_str ? scrn->scrn_device_str : "";
	node->data->screen.device =
		XtVaCreateManagedWidget("device", asciiTextWidgetClass, box,
					XtNeditType, XawtextEdit,
					XtNstring, str,
					NULL);

	command = XtVaCreateManagedWidget("videoAdaptor", toggleWidgetClass,
					  tree, XtNstate, True,
					  XtNtreeParent, box, NULL);
	adaptor = NewNode(node, command, command, node->node, NULL);
	CreateScreenAdaptor(adaptor, scrn->scrn_adaptor_lst);
	node->child = adaptor;

	command = XtVaCreateManagedWidget("Display", toggleWidgetClass,
					  tree, XtNstate, True,
					  XtNtreeParent, box, NULL);
	display = NewNode(node, command, command, node->node, NULL);
	CreateScreenDisplay(display, scrn->scrn_display_lst);
	adaptor->next = display;
    }
    else {
	command = XtCreateManagedWidget("new", commandWidgetClass, box,
					NULL, 0);
	XtAddCallback(command, XtNcallback, NewScreenCallback, (XtPointer)node);
	label = XtVaCreateManagedWidget("value", asciiTextWidgetClass, box,
					XtNeditType, XawtextEdit,
					NULL);
	node->data->screen.text = label;
    }
    if (XtIsRealized(node->treeParent))
	XtRealizeWidget(box);
    XtManageChild(box);
}

static void
NewScreenCallback(Widget w, XtPointer user_data, XtPointer call_data)
{
    TreeNode *parent, *node = (TreeNode*)user_data;
    XF86ConfScreenPtr scrn;
    Arg args[1];
    char *label;

    XtSetArg(args[0], XtNstring, &label);
    XtGetValues(node->data->screen.text, args, 1);
    if (*label == '\0')
	return;

    parent = node->parent;
    DeleteNode(node);
    scrn = (XF86ConfScreenPtr)XtCalloc(1, sizeof(XF86ConfScreenRec));
    scrn->scrn_identifier = XtNewString(label);
    XF86Config->conf_screen_lst =
	xf86addScreen(XF86Config->conf_screen_lst, scrn);

    {
	TreeNode *lay = layoutTree->child;
	Widget sme;

	/* last one is the "new" entry */
	while (lay && lay->next != NULL) {
	    /* Adjacency is the first entry */
	    TreeNode *adj = lay->child->child;

	    while (adj != NULL) {
		sme = XtCreateManagedWidget(label, smeBSBObjectClass,
					    adj->data->adjacency.menu, NULL, 0);
		XtAddCallback(sme, XtNcallback, adj->next != NULL ?
			      AdjacencyMenuCallback : NewAdjacencyCallback,
			      (XtPointer)adj);
		adj = adj->next;
	    }
	    lay = lay->next;
	}
    }

    CreateScreen(parent, scrn);

    RelayoutTree();
}

static void
ScreenDestroy(TreeNode *node)
{
    if (node->data->screen.screen) {
	int i;
	TreeNode *lay = layoutTree->child;

	/* last one is the "new" entry */
	while (lay && lay->next) {
	    /* Adjacency is the first entry */
	    TreeNode *adj = lay->child->child;
	    CompositeWidget composite;

	    while (adj) {
		TreeNode *next = adj->next;

		composite = (CompositeWidget)adj->data->adjacency.menu;

		for (i = 0; i < composite->composite.num_children; ++i)
		    if (strcmp(XtName(composite->composite.children[i]),
			       node->data->screen.screen->scrn_identifier) == 0) {
			XtDestroyWidget(composite->composite.children[i]);
			break;
		    }

		if (adj->data->adjacency.screen == node->data->screen.screen)
		    DeleteNode(adj);

		adj = next;
	    }

	    lay = lay->next;
	}

	for (i = 0; i < computer.num_screens; i++)
	    if (computer.screens[i]->screen == node->data->screen.screen) {
		config = computer.screens[i]->widget;
		RemoveDeviceCallback(NULL, NULL, NULL);
	    }

	/* for the case of screens added and removed in the expert dialog */
	xf86removeScreen(XF86Config, node->data->screen.screen);
    }
}

static void
ScreenUpdate(TreeNode *node)
{
    char *str;

    /* defautDepth */
    XtVaGetValues(node->data->screen.defaultDepth, XtNstring, &str, NULL);
    node->data->screen.screen->scrn_defaultdepth = strtoul(str, NULL, 0);

        /* defautBpp */
    XtVaGetValues(node->data->screen.defaultBpp, XtNstring, &str, NULL);
    node->data->screen.screen->scrn_defaultbpp = strtoul(str, NULL, 0);

    /* defautFbBpp */
    XtVaGetValues(node->data->screen.defaultFbBpp, XtNstring, &str, NULL);
    node->data->screen.screen->scrn_defaultfbbpp = strtoul(str, NULL, 0);


    /* XXX Monitor and Device should be changed to a menu interface */
    /* monitor */
    XtVaGetValues(node->data->screen.monitor, XtNstring, &str, NULL);
    XtFree(node->data->screen.screen->scrn_monitor_str);
    if (*str)
	node->data->screen.screen->scrn_monitor_str = XtNewString(str);
    else
	node->data->screen.screen->scrn_monitor_str = NULL;

    /* XXX Monitor and Device should be changed to a menu interface */
    /* device */
    XtVaGetValues(node->data->screen.device, XtNstring, &str, NULL);
    XtFree(node->data->screen.screen->scrn_device_str);
    if (*str)
	node->data->screen.screen->scrn_device_str = XtNewString(str);
    else
	node->data->screen.screen->scrn_device_str = NULL;
}

static void
CreateScreenAdaptor(TreeNode *parent, XF86ConfAdaptorLinkPtr lnk)
{
    TreeNode *node, *prev;
    TreeData *data;

    if ((prev = parent->child) != NULL)
	while (prev->next)
	    prev = prev->next;

    while (lnk) {
	data = (TreeData*)XtCalloc(1, sizeof(TreeData));
	data->adaptorlink.adaptorlink = lnk;
	node = NewNode(parent, NULL, NULL, parent->node, data);
	node->destroy = ScreenAdaptorDestroy;
	CreateScreenAdaptorField(node, False);
	if (parent->child == NULL)
	    parent->child = node;
	else
	    prev->next = node;
	prev = node;
	lnk = (XF86ConfAdaptorLinkPtr)(lnk->list.next);
    }
    data = (TreeData*)XtCalloc(1, sizeof(TreeData));
    node = NewNode(parent, NULL, NULL, parent->node, data);
    if (parent->child == NULL)
	parent->child = node;
    else
	prev->next = node;
    prev = node;
    CreateScreenAdaptorField(node, True);
}

static void
CreateScreenAdaptorField(TreeNode *node, Bool addnew)
{
    Widget box, command;

    box = XtVaCreateWidget("adaptor", formWidgetClass, tree,
			   XtNtreeParent, node->treeParent, NULL);
    node->node = box;

    if (!addnew) {
	XF86ConfAdaptorLinkPtr lnk = node->data->adaptorlink.adaptorlink;

	command = XtCreateManagedWidget("remove", commandWidgetClass, box,
					NULL, 0);
	XtAddCallback(command, XtNcallback, DestroyCallback, (XtPointer)node);
	(void) XtVaCreateManagedWidget("label", labelWidgetClass, box,
					XtNlabel, lnk->al_adaptor_str, NULL);
    }
    else {
	Widget sme;
	XF86ConfVideoAdaptorPtr ptr = XF86Config->conf_videoadaptor_lst;

	command = XtVaCreateManagedWidget("new", menuButtonWidgetClass, box,
					  XtNmenuName, "adaptorMenu", NULL);
	node->data->adaptorlink.menu =
	    XtVaCreatePopupShell("adaptorMenu", simpleMenuWidgetClass, box,
				 XtNleftMargin, 1, XtNrightMargin, 1,
				 XtNtopMargin, 1, XtNbottomMargin, 1,
				 NULL);
	while (ptr) {
	    sme = XtCreateManagedWidget(ptr->va_identifier, smeBSBObjectClass,
					node->data->adaptorlink.menu, NULL, 0);
	    XtAddCallback(sme, XtNcallback, NewScreenAdaptorCallback,
			  (XtPointer)node);
	    ptr = (XF86ConfVideoAdaptorPtr)(ptr->list.next);
	}
    }
    if (XtIsRealized(node->treeParent))
	XtRealizeWidget(box);
    XtManageChild(box);
}

/*ARGSUSED*/
static void
NewScreenAdaptorCallback(Widget w, XtPointer user_data, XtPointer call_data)
{
    TreeNode *parent, *node = (TreeNode*)user_data;
    XF86ConfAdaptorLinkPtr link;
    char *ident = XtName(w);

    parent = node->parent;
    DeleteNode(node);
    link = (XF86ConfAdaptorLinkPtr)XtCalloc(1, sizeof(XF86ConfAdaptorLinkRec));
    link->al_adaptor_str = XtNewString(ident);
    parent->parent->data->screen.screen->scrn_adaptor_lst =
	xf86addScreenAdaptor(parent->parent->data->screen.screen->scrn_adaptor_lst,
			     link);

    CreateScreenAdaptor(parent, link);
    RelayoutTree();
}

/*ARGUSED*/
static void
ScreenAdaptorDestroy(TreeNode *node)
{
    if (node->data->adaptorlink.adaptorlink)
	xf86removeScreenAdaptorLink(node->parent->parent->data->screen.screen,
				    node->data->adaptorlink.adaptorlink);
}

static void
CreateScreenDisplay(TreeNode *parent, XF86ConfDisplayPtr dsp)
{
    TreeNode *node, *prev;
    TreeData *data;

    if ((prev = parent->child) != NULL)
	while (prev->next)
	    prev = prev->next;

    while (dsp) {
	data = (TreeData*)XtCalloc(1, sizeof(TreeData));
	data->display.display = dsp;
	node = NewNode(parent, NULL, NULL, parent->node, data);
	node->destroy = ScreenDisplayDestroy;
	node->update = ScreenDisplayUpdate;
	CreateScreenDisplayField(node, False);
	if (parent->child == NULL)
	    parent->child = node;
	else
	    prev->next = node;
	prev = node;
	dsp = (XF86ConfDisplayPtr)(dsp->list.next);
    }
    node = NewNode(parent, NULL, NULL, parent->node, NULL);
    if (parent->child == NULL)
	parent->child = node;
    else
	prev->next = node;
    prev = node;
    CreateScreenDisplayField(node, True);
}

static void
CreateScreenDisplayField(TreeNode *node, Bool addnew)
{
    Widget box, command;

    box = XtVaCreateWidget("display", formWidgetClass, tree,
			   XtNtreeParent, node->treeParent, NULL);
    node->node = box;

    if (!addnew) {
	char *str, buf[256];
	XF86OptionPtr *options;
	XF86ConfDisplayPtr dsp = node->data->display.display;
	TreeNode *modes;

	command = XtCreateManagedWidget("remove", commandWidgetClass, box,
					NULL, 0);
	XtAddCallback(command, XtNcallback, DestroyCallback, (XtPointer)node);
	options = &(dsp->disp_option_lst);
	command = XtCreateManagedWidget("options", commandWidgetClass, box,
					NULL, 0);
	XtAddCallback(command, XtNcallback, OptionsCallback, (XtPointer)options);

	XtCreateManagedWidget("viewportL", labelWidgetClass, box, NULL, 0);
	if (dsp->disp_frameX0 != 0 || dsp->disp_frameY0 != 0)
	    XmuSnprintf(buf, sizeof(buf), "%d %d", dsp->disp_frameX0, dsp->disp_frameY0);
	else
	    *buf = '\0';
	node->data->display.viewport =
		XtVaCreateManagedWidget("viewport", asciiTextWidgetClass, box,
					XtNeditType, XawtextEdit,
					XtNstring, buf, NULL);

	XtCreateManagedWidget("virtualL", labelWidgetClass, box, NULL, 0);
	if (dsp->disp_virtualX != 0 || dsp->disp_virtualY != 0)
	    XmuSnprintf(buf, sizeof(buf), "%d %d", dsp->disp_virtualX, dsp->disp_virtualY);
	else
	    *buf = '\0';
	node->data->display.c_virtual =
		XtVaCreateManagedWidget("virtual", asciiTextWidgetClass, box,
					XtNeditType, XawtextEdit,
					XtNstring, buf, NULL);

	XtCreateManagedWidget("depthL", labelWidgetClass, box, NULL, 0);
	if (dsp->disp_depth != 0)
	    XmuSnprintf(buf, sizeof(buf), "%d", dsp->disp_depth);
	else
	    *buf = '\0';
	node->data->display.depth =
		XtVaCreateManagedWidget("depth", asciiTextWidgetClass, box,
					XtNeditType, XawtextEdit,
					XtNstring, buf, NULL);

	XtCreateManagedWidget("bppL", labelWidgetClass, box, NULL, 0);
	if (dsp->disp_bpp != 0)
	    XmuSnprintf(buf, sizeof(buf), "%d", dsp->disp_bpp);
	else
	    *buf = '\0';
	node->data->display.bpp =
		XtVaCreateManagedWidget("bpp", asciiTextWidgetClass, box,
					XtNeditType, XawtextEdit,
					XtNstring, buf, NULL);

	XtCreateManagedWidget("visualL", labelWidgetClass, box, NULL, 0);
	str = dsp->disp_visual != NULL ? dsp->disp_visual : "";
	node->data->display.visual =
		XtVaCreateManagedWidget("visual", asciiTextWidgetClass, box,
					XtNeditType, XawtextEdit,
					XtNstring, str, NULL);

	XtCreateManagedWidget("weightL", labelWidgetClass, box, NULL, 0);
	if (dsp->disp_weight.red > 0)
	    XmuSnprintf(buf, sizeof(buf), "%d %d %d",
			dsp->disp_weight.red, dsp->disp_weight.green, dsp->disp_weight.blue);
	else
	    *buf = '\0';
	node->data->display.weight =
		XtVaCreateManagedWidget("weight", asciiTextWidgetClass, box,
					XtNeditType, XawtextEdit,
					XtNstring, buf, NULL);

	XtCreateManagedWidget("blackL", labelWidgetClass, box, NULL, 0);
	if (dsp->disp_black.red >= 0)
	    XmuSnprintf(buf, sizeof(buf), "0x%04x 0x%04x 0x%04x",
			dsp->disp_black.red, dsp->disp_black.green, dsp->disp_black.blue);
	else
	    *buf = '\0';
	node->data->display.black =
		XtVaCreateManagedWidget("black", asciiTextWidgetClass, box,
					XtNeditType, XawtextEdit,
					XtNstring, buf, NULL);

	XtCreateManagedWidget("whiteL", labelWidgetClass, box, NULL, 0);
	if (dsp->disp_white.red >= 0)
	    XmuSnprintf(buf, sizeof(buf), "0x%04x 0x%04x 0x%04x",
			dsp->disp_white.red, dsp->disp_white.green, dsp->disp_white.blue);
	else
	    *buf = '\0';
	node->data->display.white =
		XtVaCreateManagedWidget("white", asciiTextWidgetClass, box,
					XtNeditType, XawtextEdit,
					XtNstring, buf, NULL);

	command = XtVaCreateManagedWidget("Modes", toggleWidgetClass, tree,
					  XtNstate, True, XtNtreeParent, box,
					  NULL);
	modes = NewNode(node, command, command, node->node, NULL);
	node->child = modes;
	CreateDisplayMode(modes, dsp->disp_mode_lst);
    }
    else {
	command = XtCreateManagedWidget("new", commandWidgetClass, box, NULL, 0);
	XtAddCallback(command, XtNcallback, NewScreenDisplayCallback,
		     (XtPointer)node);
    }
    if (XtIsRealized(node->treeParent))
	XtRealizeWidget(box);
    XtManageChild(box);
}

/*ARGSUSED*/
static void
NewScreenDisplayCallback(Widget w, XtPointer user_data, XtPointer call_data)
{
    TreeNode *parent, *node = (TreeNode*)user_data;
    XF86ConfDisplayPtr dsp;

    parent = node->parent;
    DeleteNode(node);
    dsp = (XF86ConfDisplayPtr)XtCalloc(1, sizeof(XF86ConfDisplayRec));
    dsp->disp_black.red = dsp->disp_black.green = dsp->disp_black.blue =
    dsp->disp_white.red = dsp->disp_white.green = dsp->disp_white.blue = -1;
    parent->parent->data->screen.screen->scrn_display_lst =
	xf86addScreenDisplay(parent->parent->data->screen.screen->scrn_display_lst,
			     dsp);

    CreateScreenDisplay(parent, dsp);
    RelayoutTree();
}

static void
ScreenDisplayDestroy(TreeNode *node)
{
    if (node->data->display.display)
	xf86removeScreenDisplay(node->parent->parent->data->screen.screen,
				node->data->display.display);
}

static void
ScreenDisplayUpdate(TreeNode *node)
{
    char *str, *tmp;
    int x, y;

    /* viewport */
    XtVaGetValues(node->data->display.viewport, XtNstring, &str, NULL);
    if (sscanf(str, "%d %d", &x, &y) == 2) {
	node->data->display.display->disp_frameX0 = x;
	node->data->display.display->disp_frameY0 = y;
    }

    /* virtual */
    XtVaGetValues(node->data->display.c_virtual, XtNstring, &str, NULL);
    if (sscanf(str, "%d %d", &x, &y) == 2) {
	node->data->display.display->disp_virtualX = x;
	node->data->display.display->disp_virtualY = y;
    }

    /* depth */
    XtVaGetValues(node->data->display.depth, XtNstring, &str, NULL);
    node->data->display.display->disp_depth = strtoul(str, NULL, 0);

    /* bpp */
    XtVaGetValues(node->data->display.bpp, XtNstring, &str, NULL);
    node->data->display.display->disp_bpp = strtoul(str, NULL, 0);

    /* visual */
    XtVaGetValues(node->data->display.visual, XtNstring, &str, NULL);
    XtFree(node->data->display.display->disp_visual);
    if (*str)
	node->data->display.display->disp_visual = XtNewString(str);
    else
	node->data->display.display->disp_visual = NULL;

    /* weight */
    XtVaGetValues(node->data->display.weight, XtNstring, &str, NULL);
    node->data->display.display->disp_weight.red = strtoul(str, &tmp, 0);
    if (str == tmp)
	node->data->display.display->disp_weight.red = 0;
    else {
	str = tmp;
	while (isspace(*str))
	    ++str;
	node->data->display.display->disp_weight.green = strtoul(str, &tmp, 0);
	if (str != tmp) {
	    str = tmp;
	    while (isspace(*str))
		++str;
	    node->data->display.display->disp_weight.blue = strtoul(str, &tmp, 0);
	}
    }

    /* black */
    XtVaGetValues(node->data->display.black, XtNstring, &str, NULL);
    node->data->display.display->disp_black.red = strtoul(str, &tmp, 0);
    if (str == tmp)
	node->data->display.display->disp_black.red = -1;
    else {
	str = tmp;
	while (isspace(*str))
	    ++str;
	node->data->display.display->disp_black.green = strtoul(str, &tmp, 0);
	if (str != tmp) {
	    str = tmp;
	    while (isspace(*str))
		++str;
	    node->data->display.display->disp_black.blue = strtoul(str, &tmp, 0);
	}
    }

    /* white */
    XtVaGetValues(node->data->display.white, XtNstring, &str, NULL);
    node->data->display.display->disp_white.red = strtoul(str, &tmp, 0);
    if (str == tmp)
	node->data->display.display->disp_white.red = -1;
    else {
	str = tmp;
	while (isspace(*str))
	    ++str;
	node->data->display.display->disp_white.green = strtoul(str, &tmp, 0);
	if (str != tmp) {
	    str = tmp;
	    while (isspace(*str))
		++str;
	    node->data->display.display->disp_white.blue = strtoul(str, &tmp, 0);
	}
    }
}

static void
CreateDisplayMode(TreeNode *parent, XF86ModePtr modes)
{
    TreeNode *node, *prev;
    TreeData *data;

    if ((prev = parent->child) != NULL)
	while (prev->next)
	    prev = prev->next;

    while (modes) {
	data = (TreeData*)XtCalloc(1, sizeof(TreeData));
	data->mode.mode = modes;
	node = NewNode(parent, NULL, NULL, parent->node, data);
	node->destroy = DisplayModeDestroy;
	CreateDisplayModeField(node, False);
	if (parent->child == NULL)
	    parent->child = node;
	else
	    prev->next = node;
	prev = node;
	modes = (XF86ModePtr)(modes->list.next);
    }
    data = (TreeData*)XtCalloc(1, sizeof(TreeData));
    node = NewNode(parent, NULL, NULL, parent->node, data);
    if (parent->child == NULL)
	parent->child = node;
    else
	prev->next = node;
    prev = node;
    CreateDisplayModeField(node, True);
}

static void
CreateDisplayModeField(TreeNode *node, Bool addnew)
{
    Widget box, command, text;

    box = XtVaCreateWidget("mode", formWidgetClass, tree,
			   XtNtreeParent, node->treeParent, NULL);
    node->node = box;
    if (!addnew) {
	XF86ModePtr mode = node->data->mode.mode;

	command = XtCreateManagedWidget("remove", commandWidgetClass, box,
					NULL, 0);
	XtAddCallback(command, XtNcallback, DestroyCallback,
		      (XtPointer)node);
	text = XtVaCreateManagedWidget("label", labelWidgetClass, box,
				       XtNlabel, mode->mode_name, NULL);
    }
    else {
	command = XtCreateManagedWidget("new", commandWidgetClass, box, NULL, 0);
	XtAddCallback(command, XtNcallback, NewDisplayModeCallback,
		      (XtPointer)node);
	text = XtVaCreateManagedWidget("value", asciiTextWidgetClass, box,
				       XtNeditType, XawtextEdit, NULL);
    }
    node->data->mode.text = text;
    if (node->treeParent && XtIsRealized(node->treeParent))
	XtRealizeWidget(box);
    XtManageChild(box);
}

/*ARGSUSED*/
static void
NewDisplayModeCallback(Widget w, XtPointer user_data, XtPointer call_data)
{
    TreeNode *parent, *node = (TreeNode*)user_data;
    XF86ModePtr mode;
    Arg args[1];
    char *ident;

    XtSetArg(args[0], XtNstring, &ident);
    XtGetValues(node->data->mode.text, args, 1);
    if (*ident == '\0')
	return;

    parent = node->parent;
    DeleteNode(node);
    mode = (XF86ModePtr)XtCalloc(1, sizeof(XF86ModeRec));
    mode->mode_name = XtNewString(ident);
    parent->parent->data->display.display->disp_mode_lst =
	xf86addDisplayMode(parent->parent->data->display.display->disp_mode_lst,
			   mode);

    CreateDisplayMode(parent, mode);
    RelayoutTree();
}

/*ARGUSED*/
static void
DisplayModeDestroy(TreeNode *node)
{
    if (node->data->mode.mode)
	xf86removeDisplayMode(node->parent->parent->data->display.display,
			      node->data->mode.mode);
}

/* Input */
static void
CreateInput(TreeNode *parent, XF86ConfInputPtr input)
{
    TreeNode *prev, *node;
    TreeData *data;

    if ((prev = parent->child) != NULL)
	while (prev->next)
	    prev = prev->next;

    while (input) {
	data = (TreeData*)XtCalloc(1, sizeof(TreeData));
	data->input.input = input;
	node = NewNode(parent, NULL, NULL, parent->node, data);
	node->destroy = InputDestroy;
	node->update = InputUpdate;
	CreateInputField(node, False);
	if (parent->child == NULL)
	    parent->child = node;
	else
	    prev->next = node;
	prev = node;
	input = (XF86ConfInputPtr)(input->list.next);
    }

    data = (TreeData*)XtCalloc(1, sizeof(TreeData));
    node = NewNode(parent, NULL, NULL, parent->node, data);
    CreateInputField(node, True);
    if (parent->child == NULL)
	parent->child = node;
    else
	prev->next = node;
}

static void
CreateInputField(TreeNode *node, Bool addnew)
{
    Widget box, command;

    box = XtVaCreateWidget("input", formWidgetClass, tree,
			   XtNtreeParent, node->treeParent, NULL);
    node->node = box;

    if (!addnew) {
	char *str;
	XF86OptionPtr *options;
	XF86ConfInputPtr inp = node->data->input.input;

	command = XtCreateManagedWidget("remove", commandWidgetClass, box,
					NULL, 0);
	XtAddCallback(command, XtNcallback, DestroyCallback, (XtPointer)node);
	options = &(inp->inp_option_lst);
	command = XtCreateManagedWidget("options", commandWidgetClass, box,
					NULL, 0);
	XtAddCallback(command, XtNcallback, OptionsCallback, (XtPointer)options);
	XtVaCreateManagedWidget("label", labelWidgetClass, box,
				XtNlabel, inp->inp_identifier, NULL);

	XtCreateManagedWidget("driverL", labelWidgetClass, box, NULL, 0);
	str = inp->inp_driver != NULL ? inp->inp_driver : "";
	node->data->input.text =
		XtVaCreateManagedWidget("driver", asciiTextWidgetClass, box,
					XtNeditType, XawtextEdit,
					XtNstring, str, NULL);
    }
    else {
	command = XtCreateManagedWidget("new", commandWidgetClass, box, NULL, 0);
	XtAddCallback(command, XtNcallback, NewInputCallback,
		     (XtPointer)node);
	node->data->input.text =
		XtVaCreateManagedWidget("value", asciiTextWidgetClass, box,
					XtNeditType, XawtextEdit, NULL);
    }
    if (XtIsRealized(node->treeParent))
	XtRealizeWidget(box);
    XtManageChild(box);
}

/*ARGSUSED*/
static void
NewInputCallback(Widget w, XtPointer user_data, XtPointer call_data)
{
    TreeNode *parent, *node = (TreeNode*)user_data;
    XF86ConfInputPtr input;
    Arg args[1];
    char *ident;

    XtSetArg(args[0], XtNstring, &ident);
    XtGetValues(node->data->input.text, args, 1);
    if (*ident == '\0')
	return;

    parent = node->parent;
    DeleteNode(node);
    input = (XF86ConfInputPtr)XtCalloc(1, sizeof(XF86ConfInputRec));
    input->inp_identifier = XtNewString(ident);
    XF86Config->conf_input_lst =
	xf86addInput(XF86Config->conf_input_lst, input);

    {
	TreeNode *lay = layoutTree->child;
	Widget sme;

	/* last one is the "new" entry */
	while (lay && lay->next != NULL) {
	    /* Inputref is the second entry */
	    TreeNode *iref = lay->child->next->child;

	    while (iref && iref->next)
		iref = iref->next;
	    sme = XtCreateManagedWidget(ident, smeBSBObjectClass,
					iref->data->inputref.menu, NULL, 0);
	    XtAddCallback(sme, XtNcallback, NewInputrefCallback,
			  (XtPointer)iref);
	    lay = lay->next;
	}
    }

    CreateInput(parent, input);
    RelayoutTree();
}

/*ARGUSED*/
static void
InputDestroy(TreeNode *node)
{
    if (node->data->input.input) {
	int i;
	TreeNode *lay = layoutTree->child;

	/* last one is the "new" entry */
	while (lay && lay->next) {
	    /* Inputref is the second entry */
	    TreeNode *iref = lay->child->next->child;
	    CompositeWidget composite;

	    while (iref && iref->next) {
		TreeNode *next = iref->next;

		if (iref && strcmp(iref->data->inputref.inputref->iref_inputdev_str,
				  node->data->input.input->inp_identifier) == 0)
		    DeleteNode(iref);
		iref = next;
	    }

	    composite = (CompositeWidget)iref->data->inputref.menu;

	    for (i = 0; i < composite->composite.num_children; ++i)
		if (strcmp(XtName(composite->composite.children[i]),
			   node->data->input.input->inp_identifier) == 0)
		    XtDestroyWidget(composite->composite.children[i]);

	    lay = lay->next;
	}

	for (i = 0; i < computer.num_devices; i++)
	    if ((XF86ConfInputPtr)(computer.devices[i]->config) ==
		node->data->input.input) {
		config = computer.devices[i]->widget;
		RemoveDeviceCallback(NULL, NULL, NULL);
	    }
    }
}

static void
InputUpdate(TreeNode *node)
{
    char *str;

    /* vendor */
    XtVaGetValues(node->data->input.text, XtNstring, &str, NULL);
    XtFree(node->data->input.input->inp_driver);
    if (*str)
	node->data->input.input->inp_driver = XtNewString(str);
    else
	node->data->input.input->inp_driver = NULL;
}

/* Layout */
static void
CreateLayout(TreeNode *parent, XF86ConfLayoutPtr lay)
{
    TreeNode *prev, *node;
    TreeData *data;

    if ((prev = parent->child) != NULL)
	while (prev->next)
	    prev = prev->next;

    while (lay) {
	data = (TreeData*)XtCalloc(1, sizeof(TreeData));
	data->layout.layout = lay;
	node = NewNode(parent, NULL, NULL, parent->node, data);
	node->destroy = LayoutDestroy;
	CreateLayoutField(node, False);
	if (parent->child == NULL)
	    parent->child = node;
	else
	    prev->next = node;
	prev = node;
	lay = (XF86ConfLayoutPtr)(lay->list.next);
    }

    data = (TreeData*)XtCalloc(1, sizeof(TreeData));
    node = NewNode(parent, NULL, NULL, parent->node, data);
    CreateLayoutField(node, True);
    if (parent->child == NULL)
	parent->child = node;
    else
	prev->next = node;
}

static void
CreateLayoutField(TreeNode *node, Bool addnew)
{
    Widget box, command, label;

    box = XtVaCreateWidget("layout", formWidgetClass, tree,
			   XtNtreeParent, node->treeParent, NULL);
    node->node = box;

    if (!addnew) {
	TreeNode *adjacency, *inputref;
	XF86OptionPtr *options;
	XF86ConfLayoutPtr lay = node->data->layout.layout;

	options = &(lay->lay_option_lst);
	command = XtCreateManagedWidget("remove", commandWidgetClass, box,
					NULL, 0);
	XtAddCallback(command, XtNcallback, DestroyCallback, (XtPointer)node);
	command = XtCreateManagedWidget("options", commandWidgetClass, box,
					NULL, 0);
	XtAddCallback(command, XtNcallback, OptionsCallback, (XtPointer)options);
	label = XtVaCreateManagedWidget("label", labelWidgetClass, box,
					XtNlabel, lay->lay_identifier, NULL);

	command = XtVaCreateManagedWidget("Adjacency", toggleWidgetClass, tree,
					  XtNstate, True, XtNtreeParent, box,
					  NULL);
	adjacency = NewNode(node, command, command, box, NULL);
	node->child = adjacency;
	CreateAdjacency(adjacency, lay->lay_adjacency_lst);

	command = XtVaCreateManagedWidget("Inputref", toggleWidgetClass, tree,
					  XtNstate, True, XtNtreeParent, box,
					  NULL);
	inputref = NewNode(node, command, command, box, NULL);
	adjacency->next = inputref;
	CreateInputref(inputref, lay->lay_input_lst);
    }
    else {
	command = XtCreateManagedWidget("new", commandWidgetClass, box,
					NULL, 0);
	XtAddCallback(command, XtNcallback, NewLayoutCallback, (XtPointer)node);
	label = XtVaCreateManagedWidget("value", asciiTextWidgetClass, box,
					XtNeditType, XawtextEdit,
					NULL);
	node->data->layout.text = label;
    }
    if (XtIsRealized(node->treeParent))
	XtRealizeWidget(box);
    XtManageChild(box);
}

/*ARGUSED*/
static void
LayoutDestroy(TreeNode *node)
{
    if (node->data->layout.layout)
	xf86removeLayout(XF86Config, node->data->layout.layout);
}

/*ARGSUSED*/
static void
NewLayoutCallback(Widget unused, XtPointer user_data, XtPointer call_data)
{
    TreeNode *parent, *node = (TreeNode*)user_data;
    XF86ConfLayoutPtr lay;
    Arg args[1];
    char *label;

    XtSetArg(args[0], XtNstring, &label);
    XtGetValues(node->data->layout.text, args, 1);
    if (*label == '\0')
	return;

    parent = node->parent;
    DeleteNode(node);
    lay = (XF86ConfLayoutPtr)XtCalloc(1, sizeof(XF86ConfLayoutRec));
    lay->lay_identifier = XtNewString(label);
    XF86Config->conf_layout_lst = xf86addLayout(XF86Config->conf_layout_lst, lay);

    CreateLayout(parent, lay);
    RelayoutTree();
}

static void
CreateAdjacency(TreeNode *parent, XF86ConfAdjacencyPtr adj)
{
    TreeNode *prev, *node;
    TreeData *data;

    if ((prev = parent->child) != NULL)
	while (prev->next)
	    prev = prev->next;

    while (adj) {
	data = (TreeData*)XtCalloc(1, sizeof(TreeData));
	data->adjacency.screen = adj ? adj->adj_screen : NULL;
	data->adjacency.adjacency = adj;
	node = NewNode(parent, NULL, NULL, parent->node, data);
	node->destroy = AdjacencyDestroy;
	CreateAdjacencyField(node, False);
	if (parent->child == NULL)
	    parent->child = node;
	else
	    prev->next = node;
	prev = node;
	adj = (XF86ConfAdjacencyPtr)(adj->list.next);
    }

    data = (TreeData*)XtCalloc(1, sizeof(TreeData));
    node = NewNode(parent, NULL, NULL, parent->node, data);
    CreateAdjacencyField(node, True);
    if (parent->child == NULL)
	parent->child = node;
    else
	prev->next = node;
}

static void
CreateAdjacencyField(TreeNode *node, Bool addnew)
{
    Widget box, command, sme;
    XF86ConfScreenPtr ptr = XF86Config->conf_screen_lst;

    box = XtVaCreateWidget("adjacency", formWidgetClass, tree,
			   XtNtreeParent, node->treeParent, NULL);
    node->node = box;

    node->data->adjacency.menu =
	XtVaCreatePopupShell("screenMenu", simpleMenuWidgetClass, box,
			     XtNleftMargin, 1, XtNrightMargin, 1,
			     XtNtopMargin, 1, XtNbottomMargin, 1,
			     NULL);
    while (ptr) {
	sme = XtCreateManagedWidget(ptr->scrn_identifier, smeBSBObjectClass,
				    node->data->adjacency.menu, NULL, 0);
	XtAddCallback(sme, XtNcallback, !addnew ?
		      AdjacencyMenuCallback : NewAdjacencyCallback,
		      (XtPointer)node);
	ptr = (XF86ConfScreenPtr)(ptr->list.next);
    }

    if (!addnew) {
	char buf[32];
	Cardinal width, height;
	Widget left, right, above, below, relative, absolute;
	XF86ConfAdjacencyPtr adj = node->data->adjacency.adjacency;

	command = XtCreateManagedWidget("remove", commandWidgetClass, box,
					NULL, 0);
	XtAddCallback(command, XtNcallback, DestroyCallback, (XtPointer)node);
	(void) XtVaCreateManagedWidget("label", labelWidgetClass, box,
					XtNlabel, adj->adj_screen->scrn_identifier,
					NULL);

	XtCreateManagedWidget("scrnumL", labelWidgetClass, box, NULL, 0);
	if (adj->adj_scrnum >= 0)
	    XmuSnprintf(buf, sizeof(buf), "%d", adj->adj_scrnum);
	else
	    *buf = 0;
	node->data->adjacency.scrnum =
		XtVaCreateManagedWidget("scrnum", asciiTextWidgetClass, box,
					XtNeditType, XawtextEdit,
					XtNstring, buf, NULL);
	above = XtVaCreateManagedWidget("above", toggleWidgetClass, box,
				XtNstate, adj->adj_where == CONF_ADJ_ABOVE ?
				True : False, NULL);
	XtAddCallback(above, XtNcallback, AdjacencyToggleCallback, (XtPointer)node);
	left = XtVaCreateManagedWidget("leftOf", toggleWidgetClass, box,
				XtNradioGroup, above,
				XtNstate, adj->adj_where == CONF_ADJ_LEFTOF ?
				True : False, NULL);
	XtAddCallback(left, XtNcallback, AdjacencyToggleCallback, (XtPointer)node);

	node->data->adjacency.button =
	    XtVaCreateManagedWidget("screen", menuButtonWidgetClass, box,
				    XtNmenuName, "screenMenu", NULL);

	right = XtVaCreateManagedWidget("rightOf", toggleWidgetClass, box,
				XtNradioGroup, left,
				XtNstate, adj->adj_where == CONF_ADJ_RIGHTOF ?
				True : False, NULL);
	XtAddCallback(right, XtNcallback, AdjacencyToggleCallback, (XtPointer)node);
	below = XtVaCreateManagedWidget("below", toggleWidgetClass, box,
				XtNradioGroup, right,
				XtNstate, adj->adj_where == CONF_ADJ_BELOW ?
				True : False, NULL);
	XtAddCallback(below, XtNcallback, AdjacencyToggleCallback, (XtPointer)node);
	relative = XtVaCreateManagedWidget("relative", toggleWidgetClass, box,
				XtNradioGroup, below,
				XtNstate, adj->adj_where == CONF_ADJ_RELATIVE ?
				True : False, NULL);
	XtAddCallback(relative, XtNcallback, AdjacencyToggleCallback, (XtPointer)node);
	absolute = XtVaCreateManagedWidget("absolute", toggleWidgetClass, box,
				XtNradioGroup, relative,
				XtNstate, adj->adj_where == CONF_ADJ_ABSOLUTE ?
				True : False, NULL);
	XtAddCallback(absolute, XtNcallback, AdjacencyToggleCallback, (XtPointer)node);

	XtCreateManagedWidget("adjxL", labelWidgetClass, box, NULL, 0);
	XmuSnprintf(buf, sizeof(buf), "%d", adj->adj_x);
	node->data->adjacency.adjx =
		XtVaCreateManagedWidget("adjx", asciiTextWidgetClass, box,
					XtNeditType, XawtextEdit,
					XtNstring, buf, NULL);

	XtCreateManagedWidget("adjyL", labelWidgetClass, box, NULL, 0);
	XmuSnprintf(buf, sizeof(buf), "%d", adj->adj_y);
	node->data->adjacency.adjy =
		XtVaCreateManagedWidget("adjy", asciiTextWidgetClass, box,
					XtNeditType, XawtextEdit,
					XtNstring, buf, NULL);

	XtVaGetValues(node->data->adjacency.button, XtNwidth, &width,
		      XtNheight, &height, NULL);
	if (adj->adj_where > CONF_ADJ_ABSOLUTE &&
	    adj->adj_where <= CONF_ADJ_RELATIVE)
	    XtVaSetValues(node->data->adjacency.button, XtNlabel,
			  adj->adj_refscreen, XtNwidth, width,
			  XtNheight, height, NULL);
	else
	    XtVaSetValues(node->data->adjacency.button, XtNlabel, "",
			  XtNwidth, width, XtNheight, height, NULL);
    }
    else
	XtVaCreateManagedWidget("new", menuButtonWidgetClass, box,
				XtNmenuName, "screenMenu", NULL);

    if (XtIsRealized(node->treeParent))
	XtRealizeWidget(box);
    XtManageChild(box);
}

static void
AdjacencyDestroy(TreeNode *node)
{
    if (node->data->adjacency.adjacency)
	xf86removeAdjacency(node->parent->parent->data->layout.layout,
			    node->data->adjacency.adjacency);
}

/*ARGSUSED*/
static void
NewAdjacencyCallback(Widget w, XtPointer user_data, XtPointer call_data)
{
    TreeNode *parent, *node = (TreeNode*)user_data;
    XF86ConfAdjacencyPtr adj;
    char *ident = XtName(w);

    parent = node->parent;
    DeleteNode(node);
    adj = (XF86ConfAdjacencyPtr)XtCalloc(1, sizeof(XF86ConfAdjacencyRec));
    adj->adj_screen = xf86findScreen(ident, XF86Config->conf_screen_lst);
    if (adj->adj_screen)
	adj->adj_screen_str = XtNewString(adj->adj_screen->scrn_identifier);
    parent->parent->data->layout.layout->lay_adjacency_lst =
	xf86addAdjacency(parent->parent->data->layout.layout->lay_adjacency_lst,
			 adj);

    CreateAdjacency(parent, adj);
    RelayoutTree();
}

/*ARGUSED*/
static void
AdjacencyMenuCallback(Widget w, XtPointer user_data, XtPointer call_data)
{
    TreeNode *node = (TreeNode*)user_data;
    XF86ConfAdjacencyPtr adj = node->data->adjacency.adjacency;

    XtFree(adj->adj_refscreen);
    adj->adj_refscreen = XtNewString(XtName(w));
    XtVaSetValues(node->data->adjacency.button, XtNlabel, XtName(w), NULL);
}

static void
AdjacencyToggleCallback(Widget w, XtPointer user_data, XtPointer call_data)
{
    TreeNode *node = (TreeNode*)user_data;
    XF86ConfAdjacencyPtr adj = node->data->adjacency.adjacency;
    char *x, *y;

    if ((Bool)(long)call_data == False)
	return;

    XtVaGetValues(node->data->adjacency.adjx, XtNstring, &x, NULL);
    XtVaGetValues(node->data->adjacency.adjy, XtNstring, &y, NULL);

    adj->adj_x = strtol(x, NULL, 0);
    adj->adj_y = strtol(y, NULL, 0);

    if (strcmp(XtName(w), "absolute") == 0) {
    XtVaSetValues(node->data->adjacency.button, XtNlabel, "", NULL);
	adj->adj_where = CONF_ADJ_ABSOLUTE;
	return;
    }
    if (strcmp(XtName(w), "relative") == 0)
	adj->adj_where = CONF_ADJ_RELATIVE;
    else if (strcmp(XtName(w), "leftOf") == 0)
	adj->adj_where = CONF_ADJ_LEFTOF;
    else if (strcmp(XtName(w), "rightOf") == 0)
	adj->adj_where = CONF_ADJ_RIGHTOF;
    else if (strcmp(XtName(w), "above") == 0)
	adj->adj_where = CONF_ADJ_ABOVE;
    else if (strcmp(XtName(w), "below") == 0)
	adj->adj_where = CONF_ADJ_BELOW;
}

/* Inputref */
static void
CreateInputref(TreeNode *parent, XF86ConfInputrefPtr input)
{
    TreeNode *prev, *node;
    TreeData *data;

    if ((prev = parent->child) != NULL)
	while (prev->next)
	    prev = prev->next;

    while (input) {
	data = (TreeData*)XtCalloc(1, sizeof(TreeData));
	data->inputref.inputref = input;
	node = NewNode(parent, NULL, NULL, parent->node, data);
	node->destroy = InputrefDestroy;
	CreateInputrefField(node, False);
	if (parent->child == NULL)
	    parent->child = node;
	else
	    prev->next = node;
	prev = node;
	input = (XF86ConfInputrefPtr)(input->list.next);
    }

    data = (TreeData*)XtCalloc(1, sizeof(TreeData));
    node = NewNode(parent, NULL, NULL, parent->node, data);
    CreateInputrefField(node, True);
    if (parent->child == NULL)
	parent->child = node;
    else
	prev->next = node;
}

static void
CreateInputrefField(TreeNode *node, Bool addnew)
{
    Widget box, command;

    box = XtVaCreateWidget("inputref", formWidgetClass, tree,
			   XtNtreeParent, node->treeParent, NULL);
    node->node = box;

    if (!addnew) {
	XF86OptionPtr *options;
	XF86ConfInputrefPtr inp = node->data->inputref.inputref;

	command = XtCreateManagedWidget("remove", commandWidgetClass, box,
					NULL, 0);
	XtAddCallback(command, XtNcallback, DestroyCallback, (XtPointer)node);
	options = &(inp->iref_option_lst);
	command = XtCreateManagedWidget("options", commandWidgetClass, box,
					NULL, 0);
	XtAddCallback(command, XtNcallback, OptionsCallback, (XtPointer)options);
	XtVaCreateManagedWidget("label", labelWidgetClass, box,
				XtNlabel, inp->iref_inputdev_str, NULL);
    }
    else {
	Widget sme;
	XF86ConfInputPtr ptr = XF86Config->conf_input_lst;

	XtVaCreateManagedWidget("new", menuButtonWidgetClass, box,
				XtNmenuName, "inputMenu", NULL);
	node->data->inputref.menu =
	    XtVaCreatePopupShell("inputMenu", simpleMenuWidgetClass, box,
				 XtNleftMargin, 1, XtNrightMargin, 1,
				 XtNtopMargin, 1, XtNbottomMargin, 1,
				 NULL);

	while (ptr) {
	    sme = XtCreateManagedWidget(ptr->inp_identifier, smeBSBObjectClass,
					node->data->inputref.menu, NULL, 0);
	    XtAddCallback(sme, XtNcallback, NewInputrefCallback,
			  (XtPointer)node);
	    ptr = (XF86ConfInputPtr)(ptr->list.next);
	}
    }
    if (XtIsRealized(node->treeParent))
	XtRealizeWidget(box);
    XtManageChild(box);
}

/*ARGSUSED*/
static void
NewInputrefCallback(Widget w, XtPointer user_data, XtPointer call_data)
{
    TreeNode *parent, *node = (TreeNode*)user_data;
    XF86ConfInputrefPtr input;
    char *ident = XtName(w);

    parent = node->parent;
    DeleteNode(node);
    input = (XF86ConfInputrefPtr)XtCalloc(1, sizeof(XF86ConfInputrefRec));
    input->iref_inputdev_str = XtNewString(ident);
    parent->parent->data->layout.layout->lay_input_lst =
	xf86addInputref(parent->parent->data->layout.layout->lay_input_lst, input);

    CreateInputref(parent, input);
    RelayoutTree();
}

/*ARGUSED*/
static void
InputrefDestroy(TreeNode *node)
{
    if (node->data->inputref.inputref)
	xf86removeInputRef(node->parent->parent->data->layout.layout, node->data->inputref.inputref->iref_inputdev);
}

/* Vendor */
static void
CreateVendor(TreeNode *parent, XF86ConfVendorPtr vendor)
{
    TreeNode *prev, *node;
    TreeData *data;

    if ((prev = parent->child) != NULL)
	while (prev->next)
	    prev = prev->next;

    while (vendor) {
	data = (TreeData*)XtCalloc(1, sizeof(TreeData));
	data->vendor.vendor = vendor;
	node = NewNode(parent, NULL, NULL, parent->node, data);
	node->destroy = VendorDestroy;
	CreateVendorField(node, False);
	if (parent->child == NULL)
	    parent->child = node;
	else
	    prev->next = node;
	prev = node;
	vendor = (XF86ConfVendorPtr)(vendor->list.next);
    }

    data = (TreeData*)XtCalloc(1, sizeof(TreeData));
    node = NewNode(parent, NULL, NULL, parent->node, data);
    CreateVendorField(node, True);
    if (parent->child == NULL)
	parent->child = node;
    else
	prev->next = node;
}

static void
CreateVendorField(TreeNode *node, Bool addnew)
{
    Widget box, command;

    box = XtVaCreateWidget("vendor", formWidgetClass, tree,
			   XtNtreeParent, node->treeParent, NULL);
    node->node = box;

    if (!addnew) {
	TreeNode *sub;
	XF86OptionPtr *options;
	XF86ConfVendorPtr vendor = node->data->vendor.vendor;

	command = XtCreateManagedWidget("remove", commandWidgetClass, box,
					NULL, 0);
	XtAddCallback(command, XtNcallback, DestroyCallback, (XtPointer)node);
	options = &(vendor->vnd_option_lst);
	command = XtCreateManagedWidget("options", commandWidgetClass, box,
					NULL, 0);
	XtAddCallback(command, XtNcallback, OptionsCallback, (XtPointer)options);
	XtVaCreateManagedWidget("label", labelWidgetClass, box,
				XtNlabel, vendor->vnd_identifier, NULL);

	command = XtVaCreateManagedWidget("VendSub", toggleWidgetClass, tree,
					  XtNstate, True,
					  XtNtreeParent, box,
					  NULL);
	sub = NewNode(node, command, command, box, NULL);
	node->child = sub;
	CreateVendorSub(sub, vendor->vnd_sub_lst);
    }
    else {
	command = XtCreateManagedWidget("new", commandWidgetClass, box,
					NULL, 0);
	XtAddCallback(command, XtNcallback, NewVendorCallback, (XtPointer)node);
	node->data->vendor.text =
	    XtVaCreateManagedWidget("value", asciiTextWidgetClass, box,
				    XtNeditType, XawtextEdit, NULL);
    }
    if (XtIsRealized(node->treeParent))
	XtRealizeWidget(box);
    XtManageChild(box);
}

static void
VendorDestroy(TreeNode *node)
{
    if (node->data->vendor.vendor)
	xf86removeVendor(XF86Config, node->data->vendor.vendor);
}

static void
NewVendorCallback(Widget w, XtPointer user_data, XtPointer call_data)
{
    TreeNode *parent, *node = (TreeNode*)user_data;
    XF86ConfVendorPtr vnd;
    Arg args[1];
    char *label;

    XtSetArg(args[0], XtNstring, &label);
    XtGetValues(node->data->vendor.text, args, 1);
    if (*label == '\0')
	return;

    parent = node->parent;
    DeleteNode(node);
    vnd = (XF86ConfVendorPtr)XtCalloc(1, sizeof(XF86ConfVendorRec));
    vnd->vnd_identifier = XtNewString(label);
    XF86Config->conf_vendor_lst = xf86addVendor(XF86Config->conf_vendor_lst, vnd);

    CreateVendor(parent, vnd);
    RelayoutTree();
}

/* VendorSub */
static void
CreateVendorSub(TreeNode *parent, XF86ConfVendSubPtr vendor)
{
    TreeNode *prev, *node;
    TreeData *data;

    if ((prev = parent->child) != NULL)
	while (prev->next)
	    prev = prev->next;

    while (vendor) {
	data = (TreeData*)XtCalloc(1, sizeof(TreeData));
	data->vendsub.vendsub = vendor;
	node = NewNode(parent, NULL, NULL, parent->node, data);
	node->destroy = VendorSubDestroy;
	node->update = VendorSubUpdate;
	CreateVendorSubField(node, False);
	if (parent->child == NULL)
	    parent->child = node;
	else
	    prev->next = node;
	prev = node;
	vendor = (XF86ConfVendSubPtr)(vendor->list.next);
    }

    data = (TreeData*)XtCalloc(1, sizeof(TreeData));
    node = NewNode(parent, NULL, NULL, parent->node, data);
    CreateVendorSubField(node, True);
    if (parent->child == NULL)
	parent->child = node;
    else
	prev->next = node;
}

static void
CreateVendorSubField(TreeNode *node, Bool addnew)
{
    Widget box, command;

    box = XtVaCreateWidget("vendorSub", formWidgetClass, tree,
			   XtNtreeParent, node->treeParent, NULL);
    node->node = box;

    if (!addnew) {
	XF86OptionPtr *options;
	XF86ConfVendSubPtr vendor = node->data->vendsub.vendsub;

	command = XtCreateManagedWidget("remove", commandWidgetClass, box,
					NULL, 0);
	XtAddCallback(command, XtNcallback, DestroyCallback, (XtPointer)node);
	options = &(vendor->vs_option_lst);
	command = XtCreateManagedWidget("options", commandWidgetClass, box,
					NULL, 0);
	XtAddCallback(command, XtNcallback, OptionsCallback, (XtPointer)options);
	XtVaCreateManagedWidget("label", labelWidgetClass, box,
				XtNlabel, vendor->vs_identifier, NULL);

	XtCreateManagedWidget("nameL", labelWidgetClass, box, NULL, 0);
	node->data->vendsub.text =
	    XtVaCreateManagedWidget("name", asciiTextWidgetClass, box,
				    XtNeditType, XawtextEdit, XtNstring,
				    vendor->vs_name ? vendor->vs_name : "",
				    NULL);
    }
    else {
	command = XtCreateManagedWidget("new", commandWidgetClass, box,
					NULL, 0);
	XtAddCallback(command, XtNcallback, NewVendorSubCallback, (XtPointer)node);
	node->data->vendsub.text =
	    XtVaCreateManagedWidget("value", asciiTextWidgetClass, box,
				    XtNeditType, XawtextEdit, NULL);
    }
    if (XtIsRealized(node->treeParent))
	XtRealizeWidget(box);
    XtManageChild(box);
}

static void
VendorSubDestroy(TreeNode *node)
{
    if (node->data->vendsub.vendsub)
	xf86removeVendorSub(node->parent->parent->data->vendor.vendor,
			    node->data->vendsub.vendsub);
}

static void
VendorSubUpdate(TreeNode *node)
{
    char *str;

    XtVaGetValues(node->data->vendsub.text, XtNstring, &str, NULL);
    XtFree(node->data->vendsub.vendsub->vs_name);
    if (*str)
	node->data->vendsub.vendsub->vs_name = XtNewString(str);
    else
	node->data->vendsub.vendsub->vs_name = NULL;
}

static void
NewVendorSubCallback(Widget w, XtPointer user_data, XtPointer call_data)
{
    TreeNode *parent, *node = (TreeNode*)user_data;
    XF86ConfVendSubPtr vnd;
    Arg args[1];
    char *label;

    XtSetArg(args[0], XtNstring, &label);
    XtGetValues(node->data->vendsub.text, args, 1);
    if (*label == '\0')
	return;

    parent = node->parent;
    DeleteNode(node);
    vnd = (XF86ConfVendSubPtr)XtCalloc(1, sizeof(XF86ConfVendSubRec));
    vnd->vs_identifier = XtNewString(label);
    parent->parent->data->vendor.vendor->vnd_sub_lst =
	xf86addVendorSub(parent->parent->data->vendor.vendor->vnd_sub_lst, vnd);

    CreateVendorSub(parent, vnd);
    RelayoutTree();
}

/* DRI */
static void
CreateDRI(TreeNode *parent, XF86ConfDRIPtr dri)
{
    TreeNode *node;
    TreeData *data;

    data = (TreeData*)XtCalloc(1, sizeof(TreeData));
    data->dri.dri = dri;
    node = NewNode(parent, NULL, NULL, parent->node, data);
    parent->child = node;
    node->update = DRIUpdate;
    CreateDRIField(node);
}

static void
CreateDRIField(TreeNode *node)
{
    Widget box, toggle;
    XF86ConfDRIPtr dri = node->data->dri.dri;
    TreeNode *buffers;
    char buf[32];

    box = XtVaCreateWidget("dri", formWidgetClass, tree,
			   XtNtreeParent, node->treeParent, NULL);
    node->node = box;
    XtCreateManagedWidget("nameL", labelWidgetClass, box, NULL, 0);
    node->data->dri.name =
	XtVaCreateManagedWidget("name", asciiTextWidgetClass, box,
				XtNeditType, XawtextEdit, XtNstring,
				dri->dri_group_name ? dri->dri_group_name : "",
				NULL);

    XtCreateManagedWidget("groupL", labelWidgetClass, box, NULL, 0);
    if (dri->dri_group >= 0)
	XmuSnprintf(buf, sizeof(buf), "%d", dri->dri_group);
    else
	*buf = '\0';
    node->data->dri.group =
	XtVaCreateManagedWidget("group", asciiTextWidgetClass, box,
				XtNeditType, XawtextEdit, XtNstring, buf,
				NULL);

    XtCreateManagedWidget("modeL", labelWidgetClass, box, NULL, 0);
    if (dri->dri_mode > 0)
	XmuSnprintf(buf, sizeof(buf), "0%o", dri->dri_mode);
    else
	*buf = '\0';
    node->data->dri.mode =
	XtVaCreateManagedWidget("mode", asciiTextWidgetClass, box,
				XtNeditType, XawtextEdit, XtNstring, buf,
				NULL);

    toggle = XtVaCreateManagedWidget("Buffers", toggleWidgetClass, tree,
				     XtNstate, True, XtNtreeParent, box,
				     NULL);
    buffers = NewNode(node, toggle, toggle, box, NULL);
    node->child = buffers;
    CreateBuffers(buffers, dri->dri_buffers_lst);

    if (XtIsRealized(node->treeParent))
	XtRealizeWidget(box);
    XtManageChild(box);
}

static void
DRIUpdate(TreeNode *node)
{
    char *str;

    /* name */
    XtVaGetValues(node->data->dri.name, XtNstring, &str, NULL);
    XtFree(node->data->dri.dri->dri_group_name);
    if (*str)
	node->data->dri.dri->dri_group_name = XtNewString(str);
    else
	node->data->dri.dri->dri_group_name = NULL;

    /* group */
    XtVaGetValues(node->data->dri.group, XtNstring, &str, NULL);
    if (*str)
	node->data->dri.dri->dri_group = strtoul(str, NULL, 0);
    else
	node->data->dri.dri->dri_group = -1;

    /* mode */
    XtVaGetValues(node->data->dri.mode, XtNstring, &str, NULL);
    node->data->dri.dri->dri_mode = strtoul(str, NULL, 0);
}

/* Buffers */
static void
CreateBuffers(TreeNode *parent, XF86ConfBuffersPtr buf)
{
    TreeNode *node, *prev;
    TreeData *data;

    if ((prev = parent->child) != NULL)
	while (prev->next)
	    prev = prev->next;

    while (buf) {
	data = (TreeData*)XtCalloc(1, sizeof(TreeData));
	data->buffers.buffers = buf;
	node = NewNode(parent, NULL, NULL, parent->node, data);
	node->destroy = BuffersDestroy;
	node->update = BuffersUpdate;
	CreateBuffersField(node, False);
	if (parent->child == NULL)
	    parent->child = node;
	else
	    prev->next = node;
	prev = node;

	buf = (XF86ConfBuffersPtr)(buf->list.next);
    }
    node = NewNode(parent, NULL, NULL, parent->node, NULL);
    CreateBuffersField(node, True);
    if (parent->child == NULL)
	parent->child = node;
    else
	prev->next = node;
}

static void
CreateBuffersField(TreeNode *node, Bool addnew)
{
    Widget box, command;

    box = XtVaCreateWidget("buffers", formWidgetClass, tree,
			   XtNtreeParent, node->treeParent, NULL);
    node->node = box;

    if (!addnew) {
	char str[32];
	XF86ConfBuffersPtr buf = node->data->buffers.buffers;

	command = XtCreateManagedWidget("remove", commandWidgetClass, box,
					NULL, 0);
	XtAddCallback(command, XtNcallback, DestroyCallback, (XtPointer)node);

	XtCreateManagedWidget("countL", labelWidgetClass, box, NULL, 0);
	XmuSnprintf(str, sizeof(str), "%d", buf->buf_count);
	node->data->buffers.count =
	    XtVaCreateManagedWidget("count", asciiTextWidgetClass, box,
				    XtNeditType, XawtextEdit, XtNstring, str,
				    NULL);

	XtCreateManagedWidget("sizeL", labelWidgetClass, box, NULL, 0);
	XmuSnprintf(str, sizeof(str), "%d", buf->buf_size);
	node->data->buffers.size =
	    XtVaCreateManagedWidget("size", asciiTextWidgetClass, box,
				    XtNeditType, XawtextEdit, XtNstring, str,
				    NULL);

	XtCreateManagedWidget("flagsL", labelWidgetClass, box, NULL, 0);
	node->data->buffers.flags =
	    XtVaCreateManagedWidget("flags", asciiTextWidgetClass, box,
				    XtNeditType, XawtextEdit, XtNstring,
				    buf->buf_flags ? buf->buf_flags : "",
				    NULL);
    }
    else {
	command = XtCreateManagedWidget("new", commandWidgetClass, box,
					NULL, 0);
	XtAddCallback(command, XtNcallback, NewBuffersCallback, (XtPointer)node);
    }
    if (XtIsRealized(node->treeParent))
	XtRealizeWidget(box);
    XtManageChild(box);
}

/*ARGUSED*/
static void
BuffersDestroy(TreeNode *node)
{
    if (node->data->buffers.buffers)
	xf86removeBuffers(XF86Config->conf_dri, node->data->buffers.buffers);
}

/*ARGSUSED*/
static void
NewBuffersCallback(Widget unused, XtPointer user_data, XtPointer call_data)
{
    TreeNode *parent, *node = (TreeNode*)user_data;
    XF86ConfBuffersPtr buf;

    parent = node->parent;
    DeleteNode(node);
    buf = (XF86ConfBuffersPtr)XtCalloc(1, sizeof(XF86ConfBuffersRec));
    XF86Config->conf_dri->dri_buffers_lst =
	xf86addBuffers(XF86Config->conf_dri->dri_buffers_lst, buf);

    CreateBuffers(parent, buf);
    RelayoutTree();
}

static void
BuffersUpdate(TreeNode *node)
{
    char *str;

    /* count */
    XtVaGetValues(node->data->buffers.count, XtNstring, &str, NULL);
    node->data->buffers.buffers->buf_count = strtoul(str, NULL, 0);

    /* size */
    XtVaGetValues(node->data->buffers.size, XtNstring, &str, NULL);
    node->data->buffers.buffers->buf_size = strtoul(str, NULL, 0);

    /* flags */
    XtVaGetValues(node->data->buffers.flags, XtNstring, &str, NULL);
    if (*str)
	node->data->buffers.buffers->buf_flags = XtNewString(str);
    else
	node->data->buffers.buffers->buf_flags = NULL;
}

static TreeNode *
NewNode(TreeNode *parent, Widget node, Widget toggle, Widget treeParent,
	TreeData *data)
{
    TreeNode *tree = (TreeNode*)XtCalloc(1, sizeof(TreeNode));

    tree->parent = parent;
    tree->node = node;
    if ((tree->toggle = toggle) != NULL)
	XtAddCallback(toggle, XtNcallback, ToggleCallback, (XtPointer)tree);
    tree->treeParent = treeParent;
    tree->data = data;

    return (tree);
}

static void
DeleteNode(TreeNode *node)
{
    TreeNode *ptr = node->child;

    while (ptr != NULL) {
	TreeNode *next = ptr->next;

	DeleteNode(ptr);
	ptr = next;
    }

    if (node->parent && node->parent->child == node)
	node->parent->child = node->next;
    else if (node->parent) {
	for (ptr = node->parent->child; ptr && ptr->next != node;
	     ptr = ptr->next)
	    ;
	if (ptr)
	    ptr->next = node->next;
    }

    if (node->destroy)
	(node->destroy)(node);
    if (node->data)
	XtFree((XtPointer)node->data);

    /* sets treeParent to NULL so that RelayoutTree works correctly,
     * as the tree will properly calculate it's new size.
     */
    XtVaSetValues(node->node, XtNtreeParent, NULL, NULL);

    XtDestroyWidget(node->node);
    XtFree((XtPointer)node);
}

/*ARGUSED*/
static void
DestroyCallback(Widget w, XtPointer user_data, XtPointer call_data)
{
    TreeNode *node = (TreeNode*)user_data;

    DeleteNode(node);
    RelayoutTree();
}

static void
ToggleNodeRecursive(TreeNode *node)
{
    while (node) {
	if (!XtIsRealized(node->node))
	    XtRealizeWidget(node->node);
	XtVaSetValues(node->node, XtNtreeParent, node->treeParent, NULL);
	XtManageChild(node->node);

	if (node->child && !node->toggle)
	    ToggleNodeRecursive(node->child);

	node = node->next;
    }
}

static void
ToggleNode(TreeNode *node, Bool toggle)
{
    while (node) {
	if (toggle) {
	    if (!XtIsRealized(node->node))
		XtRealizeWidget(node->node);
	    XtVaSetValues(node->node, XtNtreeParent, node->treeParent, NULL);
	    XtManageChild(node->node);

	    if (node->child && !node->toggle)
		ToggleNodeRecursive(node->child);
	}
	else {
	    if (node->child)
		ToggleNode(node->child, False);
	    XtVaSetValues(node->node, XtNtreeParent, NULL, NULL);
	    XtUnmanageChild(node->node);
	    if (node->toggle)
		XtVaSetValues(node->toggle, XtNstate, False, NULL);
	}
	node = node->next;
    }
}

/*
 * XXX This callback can show side effects in the way it is called. If
 * the structure holding the XF86OptionPtr is reallocated, a bogus pointer
 * will be passed to this callback.
 */
static void
OptionsCallback(Widget w, XtPointer user_data, XtPointer call_data)
{
    XF86OptionPtr *options = (XF86OptionPtr*)user_data;

#ifdef USE_MODULES
    OptionsPopup(options, NULL, NULL);
#else
    OptionsPopup(options);
#endif
}

static void
RelayoutTree(void)
{
    Arg args[4];
    Dimension sliderWidth, sliderHeight, canvasWidth, canvasHeight;

    XtSetArg(args[0], XtNwidth, &sliderWidth);
    XtSetArg(args[1], XtNheight, &sliderHeight);
    XtGetValues(shell, args, 2);

    XtSetArg(args[2], XtNwidth, &canvasWidth);
    XtSetArg(args[3], XtNheight, &canvasHeight);
    XtGetValues(tree, args + 2, 2);

    XtSetArg(args[0], XtNsliderWidth, sliderWidth);
    XtSetArg(args[1], XtNsliderHeight, sliderHeight);
    XtSetArg(args[2], XtNcanvasWidth, canvasWidth);
    XtSetArg(args[3], XtNcanvasHeight, canvasHeight);
    XtSetValues(panner, args, 4);
}

static void
ToggleCallback(Widget w, XtPointer user_data, XtPointer call_data)
{
    TreeNode *nodeParent = (TreeNode*)user_data;

    if (nodeParent->child) {
	if (XtIsRealized(tree))
	    XtUnmapWidget(tree);
	ToggleNode(nodeParent->child, (Bool)(long)call_data);
	RelayoutTree();
	if (XtIsRealized(tree))
	    XtMapWidget(tree);
    }
}

/*ARGSUSED*/
static void
PannerCallback(Widget w, XtPointer user_data, XtPointer call_data)
{
    Arg args[2];
    XawPannerReport *rep = (XawPannerReport *)call_data;

    XtSetArg (args[0], XtNx, -rep->slider_x);
    XtSetArg (args[1], XtNy, -rep->slider_y);
    XtSetValues(tree, args, 2);
}

/*ARGSUSED*/
static void
PortholeCallback(Widget w, XtPointer user_data, XtPointer call_data)
{
    XawPannerReport *rep = (XawPannerReport*)call_data;
    Arg args[6];
    Cardinal n = 2;

    XtSetArg (args[0], XtNsliderX, rep->slider_x);
    XtSetArg (args[1], XtNsliderY, rep->slider_y);
    if (rep->changed != (XawPRSliderX | XawPRSliderY)) {
	XtSetArg (args[2], XtNsliderWidth, rep->slider_width);
	XtSetArg (args[3], XtNsliderHeight, rep->slider_height);
	XtSetArg (args[4], XtNcanvasWidth, rep->canvas_width);
	XtSetArg (args[5], XtNcanvasHeight, rep->canvas_height);
	n = 6;
    }
    XtSetValues(panner, args, n);
}

static void
DestroyTree(TreeNode *node)
{
    while (node) {
	TreeNode *next = node->next;
	if (node->child)
	    DestroyTree(node->child);

	if (node->data)
	    XtFree((XtPointer)node->data);
	XtFree((XtPointer)node);

	node = next;
    }
}

static void
UpdateConfig(TreeNode *node)
{
    while (node) {
	if (node->child)
	    UpdateConfig(node->child);
	if (node->update)
	    (node->update)(node);
	node = node->next;
    }
}

static Bool
ExpertInitialize(void)
{
    Widget paned, vpane, close, config, files, modules, flags, video, modes,
	   monitor, device, screen, input, layout, vendor, dri;
    Arg args[4];
    Dimension width, height, canvasWidth, canvasHeight;
    TreeNode *node;

    if (expert != NULL)
	return (False);

    shell = XtCreatePopupShell("Expert", transientShellWidgetClass,
			       toplevel, NULL, 0);
    paned = XtVaCreateManagedWidget("paned", panedWidgetClass, shell,
				    XtNorientation, XtorientHorizontal, NULL);
    vpane = XtCreateManagedWidget("vpane", panedWidgetClass, paned, NULL, 0);
    panner = XtCreateManagedWidget ("panner", pannerWidgetClass, vpane, NULL, 0);
    close = XtCreateManagedWidget("close", commandWidgetClass, vpane, NULL, 0);
    XtAddCallback(close, XtNcallback, PopdownCallback, NULL);

    expert = XtCreateManagedWidget("expert", portholeWidgetClass, paned, NULL, 0);
    XtAddCallback(expert, XtNreportCallback, PortholeCallback, NULL);
    XtAddCallback(panner, XtNreportCallback, PannerCallback, NULL);
    tree = XtCreateManagedWidget("tree", treeWidgetClass, expert, NULL, 0);

    config = XtVaCreateManagedWidget(__XCONFIGFILE__, toggleWidgetClass, tree,
				     XtNstate, True, NULL);
    mainNode = NewNode(NULL, config, config, NULL, NULL);

    files = XtVaCreateManagedWidget("Files", toggleWidgetClass, tree,
				    XtNtreeParent, config, NULL);
    node = NewNode(mainNode, files, files, config, NULL);
    mainNode->child = node;
    CreateFiles(node);

    modules = XtVaCreateManagedWidget("Module", toggleWidgetClass, tree,
				      XtNtreeParent, config, NULL);
    node->next = NewNode(mainNode, modules, modules, config, NULL);
    node = node->next;
    CreateModule(node, XF86Config->conf_modules ?
		 XF86Config->conf_modules->mod_load_lst : NULL);

    flags = XtVaCreateManagedWidget("ServerFlags", commandWidgetClass, tree,
				    XtNtreeParent, config, NULL);
    node->next = NewNode(mainNode, flags, NULL, config, NULL);
    node = node->next;
    if (XF86Config->conf_flags == NULL)
	XF86Config->conf_flags = (XF86ConfFlagsPtr)
	    XtCalloc(1, sizeof(XF86ConfFlagsRec));
    XtAddCallback(flags, XtNcallback, OptionsCallback,
		  (XtPointer)&(XF86Config->conf_flags->flg_option_lst));

    video = XtVaCreateManagedWidget("VideoAdaptor", toggleWidgetClass, tree,
				     XtNtreeParent, config, NULL);
    node->next = NewNode(mainNode, video, video, config, NULL);
    node = node->next;
    CreateVideoAdaptor(node, XF86Config->conf_videoadaptor_lst);

    modes = XtVaCreateManagedWidget("Mode", toggleWidgetClass, tree,
				    XtNtreeParent, config, NULL);
    node->next = NewNode(mainNode, modes, modes, config, NULL);
    node = node->next;
    CreateModes(node, XF86Config->conf_modes_lst);

    monitor = XtVaCreateManagedWidget("Monitor", toggleWidgetClass, tree,
				      XtNtreeParent, config, NULL);
    node->next = NewNode(mainNode, monitor, monitor, config, NULL);
    node = node->next;
    CreateMonitor(monitorTree = node, XF86Config->conf_monitor_lst);

    device = XtVaCreateManagedWidget("Device", toggleWidgetClass, tree,
				     XtNtreeParent, config, NULL);
    node->next = NewNode(mainNode, device, device, config, NULL);
    node = node->next;
    CreateDevice(node, XF86Config->conf_device_lst);

    screen = XtVaCreateManagedWidget("Screen", toggleWidgetClass, tree,
				     XtNtreeParent, config, NULL);
    node->next = NewNode(mainNode, screen, screen, config, NULL);
    node = node->next;
    CreateScreen(screenTree = node, XF86Config->conf_screen_lst);

    input = XtVaCreateManagedWidget("Input", toggleWidgetClass, tree,
				    XtNtreeParent, config, NULL);
    node->next = NewNode(mainNode, input, input, config, NULL);
    node = node->next;
    CreateInput(node, XF86Config->conf_input_lst);

    layout = XtVaCreateManagedWidget("Layout", toggleWidgetClass, tree,
				     XtNtreeParent, config, NULL);
    node->next = NewNode(mainNode, layout, layout, config, NULL);
    node = node->next;
    CreateLayout(layoutTree = node, XF86Config->conf_layout_lst);

    vendor = XtVaCreateManagedWidget("Vendor", toggleWidgetClass, tree,
				     XtNtreeParent, config, NULL);
    node->next = NewNode(mainNode, vendor, vendor, config, NULL);
    node = node->next;
    CreateVendor(node, XF86Config->conf_vendor_lst);

    dri = XtVaCreateManagedWidget("DRI", toggleWidgetClass, tree,
				  XtNtreeParent, config, NULL);
    node->next = NewNode(mainNode, dri, dri, config, NULL);
    node = node->next;
    if (XF86Config->conf_dri == NULL)
	XF86Config->conf_dri = (XF86ConfDRIPtr)
	    XtCalloc(1, sizeof(XF86ConfDRIRec));
    CreateDRI(node, XF86Config->conf_dri);

    XtRealizeWidget(shell);

    XtSetArg(args[0], XtNwidth, &width);
    XtSetArg(args[1], XtNheight, &height);
    XtGetValues(shell, args, 2);
    XtSetArg(args[0], XtNwidth, width);
    XtSetArg(args[1], XtNheight, height);
    XtSetValues(expert, args, 2);

    XtSetArg(args[0], XtNsliderWidth, width);
    XtSetArg(args[1], XtNsliderHeight, height);
    XtSetArg(args[2], XtNwidth, &canvasWidth);
    XtSetArg(args[3], XtNheight, &canvasHeight);
    XtGetValues(tree, args + 2, 2);
    XtSetArg(args[2], XtNcanvasWidth, canvasWidth);
    XtSetArg(args[3], XtNcanvasHeight, canvasHeight);
    XtSetValues(panner, args, 4);

    /* needs to do the apparently NOP code bellow to correctly layout the
     * tree widget */

    /* close all open entries */
    ToggleCallback(config, mainNode, (XtPointer)0);
    /* open first level */
    ToggleCallback(config, mainNode, (XtPointer)1);

    XSetWMProtocols(DPY, XtWindow(shell), &wm_delete_window, 1);

    return (True);
}
