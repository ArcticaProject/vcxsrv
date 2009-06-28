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
#include <X11/Shell.h>
#include <X11/Xaw/AsciiText.h>
#include <X11/Xaw/Command.h>
#include <X11/Xaw/Form.h>
#include <X11/Xaw/Paned.h>

#include <X11/Xaw/Text.h>
#include <X11/Xaw/TextSinkP.h>
#include <X11/Xaw/TextSrcP.h>
#include <X11/Xmu/SysUtil.h>
#include <X11/Xmu/Xmu.h>
#include <stdlib.h>		/* for bsearch() */
#include <ctype.h>
#include "help.h"
#include "options.h"

/*
 * Prototypes
 */
static void CloseCallback(Widget, XtPointer, XtPointer);
static void StartHelp(void);
void Html_ModeStart(Widget);

/*
 * Initialization
 */
static Widget shell, text;
static Bool popped_up = False;

/*
 * Implementation
 */
void
Help(char *topic)
{
    Widget source;
    char *str = NULL;
    Bool error = False;
    static char *def_text = "<h2>Help Error</h2>"
			    "No help available for the topic <b>%s</b>.";
    XtResource resource = {
	NULL, "HelpMessage", XtRString, sizeof(char*),
	0, XtRString, NULL
    };

    StartHelp();
    source = XawTextGetSource(text);
    XawTextSourceClearEntities(source, 0,
			       XawTextSourceScan(source, 0, XawstAll,
						 XawsdRight, 1, True));
    if (topic != NULL) {
	resource.resource_name = topic;
	XtGetApplicationResources(shell, (XtPointer)&str,
				  &resource, 1, NULL, 0);
    }
    if (str == NULL) {
	int len;

	error = True;
	if (topic == NULL)
	    topic = "(null argument)";
	str = XtMalloc(len  = strlen(topic) + strlen(def_text) + 1);
	XmuSnprintf(str, len, def_text, topic);
    }
    XtVaSetValues(text, XtNstring, str, NULL);
    if (error)
	XtFree(str);

    Html_ModeStart(source);
    _XawTextBuildLineTable((TextWidget)text,
			       XawTextTopPosition(text), True);
    XawTextDisplay(text);
    if (popped_up == False) {
	popped_up = True;
	XtPopup(shell, XtGrabNone);
	XtSetKeyboardFocus(shell, text);
    }
}

static void
StartHelp(void)
{
    static XtResource resource = {
	"properties", "Properties", XtRString, sizeof(char*),
	0, XtRString, NULL
    };

    if (shell == NULL) {
	Widget pane, commands, close;
	char *props;
	XawTextPropertyList *propl;

	shell = XtCreatePopupShell("help", transientShellWidgetClass,
				   toplevel, NULL, 0);
	pane = XtCreateManagedWidget("pane", panedWidgetClass,
				     shell, NULL, 0);
	text = XtVaCreateManagedWidget("text", asciiTextWidgetClass,
				       pane, XtNeditType, XawtextRead, NULL);
	commands = XtCreateManagedWidget("commands", formWidgetClass, pane,
					 NULL, 0);
	close = XtCreateManagedWidget("close", commandWidgetClass,
				      commands, NULL, 0);
	XtAddCallback(close, XtNcallback, CloseCallback, NULL);
	XtRealizeWidget(shell);
	XSetWMProtocols(DPY, XtWindow(shell), &wm_delete_window, 1);
	XtGetApplicationResources(text, (XtPointer)&props,
				  &resource, 1, NULL, 0);
	propl = XawTextSinkConvertPropertyList("html", props,
					       toplevel->core.screen,
					       toplevel->core.colormap,
					       toplevel->core.depth);
	XtVaSetValues(XawTextGetSink(text), XawNtextProperties, propl, NULL);
    }
}

/*ARGSUSED*/
static void
CloseCallback(Widget w, XtPointer user_data, XtPointer call_data)
{
    XtPopdown(shell);
    popped_up = False;
}

/*ARGSUSED*/
void
HelpCancelAction(Widget w, XEvent *event, String *params, Cardinal *num_params)
{
    CloseCallback(w, NULL, NULL);
}


/* bellow is a modified version of the html-mode.c I wrote for xedit
 * (at least) temporarily dead.
 */

/*
 * Copyright (c) 1999 by The XFree86 Project, Inc.
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
 * THE XFREE86 PROJECT BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
 * OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * Except as contained in this notice, the name of the XFree86 Project shall
 * not be used in advertising or otherwise to promote the sale, use or other
 * dealings in this Software without prior written authorization from the
 * XFree86 Project.
 *
 * Author: Paulo César Pereira de Andrade
 */

#define Html_Peek(parser)	((parser)->next)

/*
 * Types
 */
typedef struct _Html_Parser Html_Parser;
typedef struct _Html_Item Html_Item;

typedef struct _Html_TagInfo {
    char *name;
    unsigned int entity	: 1;	/* it changes the type of the text */
    unsigned int nest	: 1;	/* does not close tags automatically */
    unsigned int end	: 1;	/* need a close markup */
    unsigned int adnl	: 1;	/* add newline before/after tag contents */
    unsigned int para	: 1;	/* changes the paragraph formatting */
    unsigned long mask;	/* enforce use of attributes of this tag-info */
    unsigned long xlfd_mask;
    void (*parse_args)(Html_Parser*, Html_Item*);
    XawTextProperty *override;
    XrmQuark ident;
} Html_TagInfo;

struct _Html_Item {
    XrmQuark ident;
    XawTextPosition start, end;
    Html_TagInfo *info;

    XawTextProperty *combine;
    Bool override;
    int li;

    XtPointer replace;

    Html_Item *parent, *child, *next;
};

struct _Html_Parser {
    Widget source;
    XawTextBlock block, replace;
    XawTextPosition position, offset, start, end, last;
    XrmQuark quark;
    int i, ch, next;
    Html_Item *item, *head;
    XmuScanline *mask;
    int space, pre, adnl, list, desc, column;
    Bool spc;
    XawTextBlock *entity;

    Pixel alink;
};

typedef struct _Html_SourceInfo Html_SourceInfo;
struct _Html_SourceInfo {
    Widget source;
    XawTextBlock block;
    XawTextPosition last;
    Html_SourceInfo *next;
};

/*
 * Proptotypes
 */
void Html_ModeEnd(Widget);
static void Html_ModeInit(void);
static void Html_ParseCallback(Widget, XtPointer, XtPointer);
static Html_TagInfo *Html_GetInfo(char*);
static int Html_Get(Html_Parser*);
static int Html_Parse1(Html_Parser*);
static int Html_Parse2(Html_Parser*);
static void Html_ParseTag(Html_Parser*);
static void Html_Commit(Html_Parser*);
static void Html_AddEntities(Html_Parser*, Html_Item*);

static int Html_Put(Html_Parser*, int);
static void Html_Puts(Html_Parser*, char*);
static int Html_Format1(Html_Parser*);
static int Html_Format2(Html_Parser*);
static int Html_Format3(Html_Parser*);
static void Html_FormatTag(Html_Parser*);

static void Html_AArgs(Html_Parser*, Html_Item*);
static void Html_FontArgs(Html_Parser*, Html_Item*);

/*
 * Initialization
 */
static XrmQuark
	Qbr,
	Qdefault,
	Qdd,
	Qdl,
	Qdt,
	Qentity,
	Qetag,
	Qhide,
	Qli,
	Qol,
	Qp,
	Qpre,
	Qspace,
	Qtag,
	Qul;

static Html_TagInfo tag_info[] = {
    {"a",	1, 0, 1, 0, 0,
     0, 0,
     Html_AArgs},
    {"address",	1, 0, 1, 0, 0,
     0, XAW_TPROP_SLANT,
     },
    {"b",	1, 0, 1, 0, 0,
     0, XAW_TPROP_WEIGHT,
     },
    {"blockquote", 0, 1, 1, 1, 1,
     0, 0,
     },
    {"body",	0, 0, 1, 0, 0,
     0, 0,
     },
    {"br",	0, 0, 0, 0, 0,
     },
    {"code",	1, 0, 1, 0, 0,
     0, XAW_TPROP_FAMILY | XAW_TPROP_PIXELSIZE,
     },
    {"dd",	0, 1, 1, 0, 1,
     0, 0},
    {"dl",	0, 1, 1, 0, 0,
     0, 0,
     },
    {"dt",	0, 0, 1, 0, 0,
     0, 0},
    {"em",	1, 0, 1, 0, 0,
     0, XAW_TPROP_SLANT,
     },
    {"font",	1, 1, 1, 0, 0,
     0, 0,
     Html_FontArgs},
    {"h1",	1, 0, 1, 1, 0,
     0, XAW_TPROP_WEIGHT | XAW_TPROP_PIXELSIZE,
     },
    {"h2",	1, 0, 1, 1, 0,
     0, XAW_TPROP_WEIGHT | XAW_TPROP_PIXELSIZE,
     },
    {"h3",	1, 0, 1, 1, 0,
     0, XAW_TPROP_WEIGHT | XAW_TPROP_PIXELSIZE,
     },
    {"h4",	1, 0, 1, 1, 0,
     0, XAW_TPROP_WEIGHT | XAW_TPROP_PIXELSIZE,
     },
    {"h5",	1, 0, 1, 1, 0,
     0, XAW_TPROP_WEIGHT | XAW_TPROP_PIXELSIZE,
     },
    {"h6",	1, 0, 1, 1, 0,
     0, XAW_TPROP_WEIGHT | XAW_TPROP_PIXELSIZE,
     },
    {"head",	0, 0, 1, 0, 0,
     0, 0,
     },
    {"html",	0, 0, 1, 0, 0,
     0, 0,
     },
    {"i",	1, 0, 1, 0, 0,
     0, XAW_TPROP_SLANT,
     },
    {"kbd",	1, 0, 1, 0, 0,
     0, XAW_TPROP_FAMILY | XAW_TPROP_PIXELSIZE,
     },
    {"li",	0, 0, 0, 0, 0,
     0, 0},
    {"ol",	0, 1, 1, 0, 1,
     0, 0,
     },
    {"p",	0, 0, 0, 1, 0,
     },
    {"pre",	1, 0, 1, 1, 0,
     0, XAW_TPROP_FAMILY | XAW_TPROP_PIXELSIZE,
     },
    {"samp",	1, 0, 1, 0, 0,
     0, XAW_TPROP_FAMILY | XAW_TPROP_PIXELSIZE,
     },
    {"strong",	1, 0, 1, 0, 0,
     0, XAW_TPROP_WEIGHT,
     },
    {"tt",	1, 0, 1, 0, 0,
     0, XAW_TPROP_FAMILY | XAW_TPROP_PIXELSIZE,
     },
    {"ul",	0, 1, 1, 0, 1,
     0, 0,
     },
};

static char *pnl = "<p>\n", *nlpnl = "\n<p>\n";
static Html_SourceInfo *source_info;

/*
 * Implementation
 */
static char *
Html_GetText(Widget src, XawTextPosition position)
{
    char *result, *tempResult;
    XawTextPosition offset = 0;
    XawTextBlock text;

    tempResult = result = XtMalloc((unsigned)(position + 1));

    while (offset < position) {
	offset = XawTextSourceRead(src, offset, &text, position - offset);
	if (!text.length)
	    break;
	memcpy(tempResult, text.ptr, (unsigned)text.length);
	tempResult += text.length;
    }

    *tempResult = '\0';

    return (result);
}

void
Html_ModeStart(Widget src)
{
    Html_Parser *parser = XtNew(Html_Parser);
    Html_Item *next, *item;
    XColor color, exact;
    Html_SourceInfo *info = XtNew(Html_SourceInfo);

    if (XAllocNamedColor(XtDisplay(toplevel), toplevel->core.colormap,
			 "blue", &color, &exact))
	parser->alink = color.pixel;
    else
	parser->alink = 0L;

    XtVaSetValues(src, XtNeditType, XawtextEdit, NULL);

    Html_ModeInit();

    /* initialize parser state */
    parser->source = src;
    parser->position = XawTextSourceRead(parser->source, 0,
					 &parser->block, 4096);
    parser->replace.ptr = NULL;
    parser->replace.firstPos = 0;
    parser->replace.length = 0;
    parser->replace.format = FMT8BIT;
    parser->offset = -1;
    parser->quark = NULLQUARK;
    parser->i = 0;
    parser->i = parser->ch = parser->next = 0;
    parser->last = XawTextSourceScan(src, 0, XawstAll, XawsdRight, 1, 1);
    if (parser->block.length == 0)
	parser->ch = parser->next = EOF;
    else
	(void)Html_Get(parser);
    parser->pre = 0;
    parser->adnl = 1;
    parser->list = parser->desc = parser->column = 0;
    parser->spc = True;

    info->source = src;
    info->block.ptr = Html_GetText(src, parser->last);
    info->block.length = parser->last;
    info->block.format = FMT8BIT;
    info->block.firstPos = 0;
    info->next = NULL;
    if (source_info == NULL)
	source_info = info;
    else {
	Html_SourceInfo *tmp = source_info;

	while (tmp->next)
	    tmp = tmp->next;
	tmp->next = info;
    }

    while (Html_Format1(parser) != EOF)
	;
    XawTextSourceReplace(parser->source, 0, parser->last, &parser->replace);
    XtFree(parser->replace.ptr);

    /* re-initialize parser state */
    parser->position = XawTextSourceRead(parser->source, 0,
					 &parser->block, 4096);
    parser->offset = -1;
    parser->quark = NULLQUARK;
    parser->i = parser->ch = parser->next = 0;
    parser->last = XawTextSourceScan(src, 0, XawstAll, XawsdRight, 1, 1);
    info->last = parser->last;
    if (parser->block.length == 0)
	parser->ch = parser->next = EOF;
    else
	(void)Html_Get(parser);
    parser->adnl = 1;
    parser->list = parser->desc = parser->column = 0;
    parser->spc = True;
    parser->head = parser->item = NULL;

    parser->mask = XmuNewScanline(0, 0, 0);

    /* build html structure information */
    while (Html_Parse1(parser) != EOF)
	;

    /* create top level entity mask */
    (void)XmuScanlineNot(parser->mask, 0, parser->last);

    item = parser->item;
    while (item) {
	next = item->next;
	Html_AddEntities(parser, item);
	if (item->combine)
	    XtFree((XtPointer)item->combine);
	XtFree((XtPointer)item);
	item = next;
    }
    XmuDestroyScanline(parser->mask);

    XtVaSetValues(src, XtNeditType, XawtextRead, NULL);

    XtFree((XtPointer)parser);

    /* add callbacks for interactive changes */
    XtAddCallback(src, XtNpropertyCallback, Html_ParseCallback, NULL);
}

void
Html_ModeEnd(Widget src)
{
    Html_SourceInfo *info, *pinfo;

    XtRemoveCallback(src, XtNpropertyCallback, Html_ParseCallback, NULL);
    for (pinfo = info = source_info; info; pinfo = info, info = info->next)
	if (info->source == src)
	    break;

    if (info == NULL)
	return;

    XawTextSourceClearEntities(src, 0, info->last);
    XtVaSetValues(src, XtNeditType, XawtextEdit, NULL);
    XawTextSourceReplace(src, 0, info->last, &info->block);
    XtVaSetValues(src, XtNeditType, XawtextRead, NULL);

    if (info == source_info)
	source_info = source_info->next;
    else
	pinfo->next = info->next;
    XtFree(info->block.ptr);
    XtFree((XtPointer)info);
}

static void
Html_ParseCallback(Widget w, XtPointer client_data, XtPointer call_data)
{
}

static int
bcmp_tag_info(_Xconst void *left, _Xconst void *right)
{
    return (strcmp((char*)left, ((Html_TagInfo*)right)->name));
}

static Html_TagInfo *
Html_GetInfo(char *name)
{
    return (bsearch(name, tag_info, sizeof(tag_info) / sizeof(tag_info[0]),
		    sizeof(Html_TagInfo), bcmp_tag_info));
}

static int
Html_Get(Html_Parser *parser)
{
    if (parser->ch == EOF)
	return (EOF);
    if (parser->i >= parser->block.length) {
	parser->i = 0;
	parser->position = XawTextSourceRead(parser->source, parser->position,
					     &parser->block, 4096);
    }
    parser->ch = parser->next;
    if (parser->block.length == 0)
	parser->next = EOF;
    else
	parser->next = (unsigned char)parser->block.ptr[parser->i++];
    parser->offset++;

    return (parser->ch);
}

static void
Html_ModeInit(void)
{
    static int initialized;
    int i;

    if (initialized)
	return;

    Qbr			= XrmPermStringToQuark("br");
    Qdd			= XrmPermStringToQuark("dd");
    Qdefault		= XrmPermStringToQuark("default");
    Qdl			= XrmPermStringToQuark("dl");
    Qdt			= XrmPermStringToQuark("dt");
    Qentity		= XrmPermStringToQuark("entity");
    Qetag		= XrmPermStringToQuark("/tag");
    Qhide		= XrmPermStringToQuark("hide");
    Qli			= XrmPermStringToQuark("li");
    Qol			= XrmPermStringToQuark("ol");
    Qp			= XrmPermStringToQuark("p");
    Qpre		= XrmPermStringToQuark("pre");
    Qspace		= XrmPermStringToQuark("space");
    Qtag		= XrmPermStringToQuark("tag");
    Qul			= XrmPermStringToQuark("ul");

    for (i = 0; i < sizeof(tag_info) / sizeof(tag_info[0]); i++)
	tag_info[i].ident = XrmPermStringToQuark(tag_info[i].name);

    initialized = True;
}

/************************************************************************/
/* PARSE								*/
/************************************************************************/
static void
Html_AddEntities(Html_Parser *parser, Html_Item *item)
{
    Html_Item *parent, *next, *child = item->child;
    XmuSegment segment, *ent;
    XmuScanline *mask = XmuNewScanline(0, 0, 0);
    XawTextProperty *tprop, *property = NULL;
    Widget sink;
    Bool changed = False;

    /* combine properties */
    if (item->info &&
	(item->info->entity ||
	 (item->parent && item->parent->ident != item->parent->info->ident))) {
	sink = XawTextGetSink(text);
	parent = item->parent;
	property = XawTextSinkCopyProperty(sink, item->ident);
	property->mask = item->info->mask;
	property->xlfd_mask = item->info->xlfd_mask;
	if (parent) {
	    (void)XawTextSinkCombineProperty(sink, property,
			XawTextSinkGetProperty(sink, parent->ident), False);
	    if (item->combine && parent->combine)
		(void)XawTextSinkCombineProperty(sink, item->combine,
					 	 parent->combine,
						 item->override);
	}
	if (item->combine)
	    XawTextSinkCombineProperty(sink, property, item->combine, True);
	tprop = property;
	property = XawTextSinkAddProperty(sink, property);
	XtFree((XtPointer)tprop);
	if (property && item->ident != property->identifier) {
	    item->ident = property->identifier;
	    changed = True;
	}
    }

    if (item->end < 0) {
	if (item->next)
	    item->end = item->next->start;
	else if (item->parent)
	    item->end = item->parent->end;
	else
	    item->end = parser->last;
    }

    while (child) {
	next = child->next;
	segment.x1 = child->start;
	segment.x2 = child->end;
	(void)XmuScanlineOrSegment(mask, &segment);
	Html_AddEntities(parser, child);
	if (child->combine)
	    XtFree((XtPointer)child->combine);
	XtFree((XtPointer)child);
	child = next;
    }

    /* build entity mask */
    (void)XmuScanlineNot(mask, item->start, item->end);
    (void)XmuScanlineAnd(mask, parser->mask);

    /* add entities */
    if (item->info && changed) {
	for (ent = mask->segment; ent; ent = ent->next)
	    (void)XawTextSourceAddEntity(parser->source, 0, 0, NULL, ent->x1,
					 ent->x2 - ent->x1, item->ident);
    }
    else if (item->info == NULL)
	(void)XawTextSourceAddEntity(parser->source, 0,
				     XAW_TENTF_READ | XAW_TENTF_REPLACE,
				     item->replace, item->start,
				     item->end - item->start,
				     item->parent->ident);

    /* set mask for parent entities */
    (void)XmuScanlineOr(parser->mask, mask);
    XmuDestroyScanline(mask);

#if 0
    if (item->info && item->info->para) {
	XawTextSourceSetParagraph(parser->source, item->start, item->end,
				  40,	/* arbitrary value, for testing */
				  0, 0);
    }
#endif
}

static void
Html_Commit(Html_Parser *parser)
{
    XawTextPosition position;
    int length;

    position = parser->start;
    length = parser->end - parser->start;
    if (position < 0) {
	length += position;
	position = 0;
    }
    if (position + length > parser->last + 1)
	length -= (position + length) - parser->last + 1;

    if (parser->quark != Qdefault && parser->quark != NULLQUARK && length > 0) {
	XmuSegment segment;
	Html_Item *head = parser->head;
	XrmQuark quark = parser->quark;

	parser->quark = Qdefault;

	if (quark == Qli && head &&
	    (head->info->ident == Qol || head->info->ident == Qul)) {
	    if (parser->head == NULL || head->info->ident != Qol)
		XawTextSourceAddEntity(parser->source, 0, /*XAW_TENT_BULLET,*/
				       XAW_TENTF_HIDE, NULL,
				       position, length, Qli);
	    else
		XawTextSourceAddEntity(parser->source, 0, /*XAW_TENT_LITEM,*/
				       XAW_TENTF_HIDE,
				       (XtPointer)(long)head->li++,
				       position, length, Qli);
	}
	else if (quark == Qhide)
	    XawTextSourceAddEntity(parser->source, 0, XAW_TENTF_HIDE, NULL,
				   position, length, quark);
	else if (quark == Qentity) {
	    if (head && head->end == -1) {
		Html_Item *item, *it;

		item = XtNew(Html_Item);
		item->ident = Qentity;
		item->start = position;
		item->end = position + length;
		item->info = NULL;
		item->combine = NULL;
		item->override = False;
		item->replace = (XtPointer)parser->entity;
		item->child = item->next = NULL;

		it = head->child;

		item->parent = head;
		if (it == NULL)
		    head->child = item;
		else {
		    while (it->next)
			it = it->next;
		    it->next = item;
		}

		return;
	    }
	    XawTextSourceAddEntity(parser->source, 0,
				   XAW_TENTF_READ | XAW_TENTF_REPLACE,
				   (XtPointer)parser->entity,
				   position, length, Qentity);
	}

	segment.x1 = position;
	segment.x2 = position + length;
	(void)XmuScanlineOrSegment(parser->mask, &segment);
    }
}

static void
Html_ParseTag(Html_Parser *parser)
{
    int ch, sz;
    char buf[32];
    Html_TagInfo *info;
    Html_Item *item = NULL;
    XawTextPosition offset = parser->offset - 1;

    switch (Html_Peek(parser)) {
	case '!':
	    (void)Html_Get(parser);			/* eat `!' */
	    if (Html_Peek(parser) == '-') {
		/* comment */
		(void)Html_Get(parser);			/* eat `-' */
		if (Html_Peek(parser) == '-') {
		    int count = 0;

		    (void)Html_Get(parser);
		    while ((ch = Html_Peek(parser)) != EOF) {
			if (ch == '>' && count >= 2)
			    break;
			else if (ch == '-')
			    ++count;
			else
			    count = 0;
			(void)Html_Get(parser);
		    }
		}
	    }
	    break;
	case '?':
	    break;
	case '/':
	    (void)Html_Get(parser);			/* eat `/' */
	    sz = 0;
	    while (isalnum(Html_Peek(parser)) &&
		   ((sz + 1) < sizeof(buf)))
		buf[sz++] = tolower(Html_Get(parser));
	    buf[sz] = '\0';
	    if ((info = Html_GetInfo(buf)) != NULL) {
		if (parser->head) {
		    Html_Item *it = parser->head;

		    while (it) {
			if (it->info == info)
			    break;
			it = it->parent;
		    }

		    if (it) {
			if (it == parser->head)
			    parser->head->end = offset;
			else {
			    it->end = offset;
			    do {
				parser->head->end = offset;
				parser->head = parser->head->parent;
			    } while (parser->head != it);
			}
			if (parser->head->parent)
			    parser->head = parser->head->parent;
			else
			    parser->head = parser->item;
		    }
		}
	    }
	    break;
	default:
	    sz = 0;
	    while (isalnum(Html_Peek(parser)) &&
		   ((sz + 1) < sizeof(buf)))
		buf[sz++] = tolower(Html_Get(parser));
	    buf[sz] = '\0';
	    if ((info = Html_GetInfo(buf)) != NULL) {
		if (info->end == False) {
		    if (info->ident == Qli)
			parser->quark = Qli;
		    if (!info->para)
			break;	/* no more processing required */
		}
		item = XtNew(Html_Item);
		item->info = info;
		item->ident = item->info->ident;
		item->combine = NULL;
		item->override = False;
		item->start = item->end = -1;
		if (info->ident == Qol)
		    item->li = 1;
		else
		    item->li = 0;
		item->parent = item->child = item->next = NULL;
		if (parser->item == NULL)
		    parser->item = parser->head = item;
		else if (parser->head->end == -1) {
		    if (parser->head->info != item->info || info->nest) {
			Html_Item *it = parser->head;

			/* first, see if we need to close a long list of tags */
			if (info->ident == Qdd) {
			    if (parser->head &&
				parser->head->info->ident == Qdt) {
				parser->head->end = offset;
				parser->head = parser->head->parent;
			    }
			}
			else if (info->ident == Qdt) {
			    if (parser->head &&
				parser->head->info->ident == Qdd) {
				parser->head->end = offset;
				parser->head = parser->head->parent;
			    }
			}
			else if (!info->nest) {
			    while (it) {
				if (it->info == info || it->info->end)
				    break;
				it = it->parent;
			    }
			    if (it) {
				/* close the items */
				 while (parser->head != it) {
				    if (parser->head->info->ident == Qpre)
					--parser->pre;
				    parser->head->end = offset;
				    parser->head = parser->head->parent;
				}
			    }
			}

			/* add child item */
			it = parser->head->child;

			item->parent = parser->head;
			if (it == NULL)
			    parser->head->child = item;
			else {
			    while (it->next)
				it = it->next;
			    it->next = item;
			}
			parser->head = item;
		    }
		    else {
			/* close the `head' item and start a new one */
			Html_Item *it;

			parser->head->end = offset;
			if (parser->head->parent)
			    parser->head = parser->head->parent;
			else
			    parser->head = parser->item;

			if ((it = parser->head->child) != NULL) {
			    item->parent = parser->head;
			    while (it->next)
				it = it->next;
			    it->next = item;
			    parser->head = item;
			}
			else {
			    parser->head->child = item;
			    parser->head = item;
			}
		    }
		}
		else {
		    /* this is not common, but handle it */
		    Html_Item *it = parser->item;

		    while (it->next)
			it = it->next;
		    it->next = item;
		    parser->head = item;
		}
		if (info->parse_args)
		    (info->parse_args)(parser, item);
	    }
	    break;
    }

    /* skip anything not processed */
    while ((ch = Html_Peek(parser)) != '>' && ch != EOF)
	(void)Html_Get(parser);
    if (item && item->start == -1)
	item->start = parser->offset + 1;
}

/* tags */
static int
Html_Parse2(Html_Parser *parser)
{
    int ch;

    for (;;) {
	if ((ch = Html_Get(parser)) == '<') {
	    parser->end = parser->offset - 1;
	    Html_Commit(parser);
	    parser->quark = Qhide;
	    parser->start = parser->end;

	    Html_ParseTag(parser);

	    (void)Html_Get(parser);	/* eat `>' */
	    parser->end = parser->offset;
	    Html_Commit(parser);
	}
	else
	    return (ch);
    }
    /*NOTREACHED*/
}

/* entities */
static int
Html_Parse1(Html_Parser *parser)
{
    static XawTextBlock *entities[256];
    static char chars[256];
    int ch;

    for (;;) {
	if ((ch = Html_Parse2(parser)) == EOF)
	    return (EOF);

	if (ch == '&') {
	    unsigned char idx = '?';
	    char buf[32];
	    int sz = 0;

	    /* the string comparisons need a big optmization! */
	    parser->end = parser->offset - 1;
	    Html_Commit(parser);
	    parser->start = parser->end;
	    while ((ch = Html_Peek(parser)) != ';'
		   && ch != EOF && !isspace(ch)) {
		ch = Html_Get(parser);
		if (sz + 1 < sizeof(buf))
		    buf[sz++] = ch;
	    }
	    buf[sz] = '\0';
	    if (ch == ';')
		(void)Html_Get(parser);
	    if (sz == 0)
		idx = '&';
	    else if (strcasecmp(buf, "lt") == 0)
		idx = '<';
	    else if (strcasecmp(buf, "gt") == 0)
		idx = '>';
	    else if (strcasecmp(buf, "nbsp") == 0)
		idx = ' ';
	    else if (strcasecmp(buf, "amp") == 0)
		idx = '&';
	    else if (strcasecmp(buf, "quot") == 0)
		idx = '"';
	    else if (*buf == '#') {
		if (sz == 1)
		    idx = '#';
		else {
		    char *tmp;

		    idx = strtol(buf + 1, &tmp, 10);
		    if (*tmp)
			idx = '?';
		}
	    }
	    else if (strcmp(buf + 1, "acute") == 0) {
		switch (*buf) {
		    case 'a': idx = 0xe1; break;	case 'e': idx = 0xe9; break;
		    case 'i': idx = 0xed; break;	case 'o': idx = 0xf3; break;
		    case 'u': idx = 0xfa; break;	case 'A': idx = 0xc1; break;
		    case 'E': idx = 0xc9; break;	case 'I': idx = 0xcd; break;
		    case 'O': idx = 0xd3; break;	case 'U': idx = 0xda; break;
		    case 'y': idx = 0xfd; break;	case 'Y': idx = 0xdd; break;
		}
	    }
	    else if (strcmp(buf + 1, "grave") == 0) {
		switch (*buf) {
		    case 'a': idx = 0xe0; break;	case 'e': idx = 0xe8; break;
		    case 'i': idx = 0xec; break;	case 'o': idx = 0xf2; break;
		    case 'u': idx = 0xf9; break;	case 'A': idx = 0xc0; break;
		    case 'E': idx = 0xc8; break;	case 'I': idx = 0xcc; break;
		    case 'O': idx = 0xd2; break;	case 'U': idx = 0xd9; break;
		}
	    }
	    else if (strcmp(buf + 1, "tilde") == 0) {
		switch (*buf) {
		    case 'a': idx = 0xe3; break;	case 'o': idx = 0xf5; break;
		    case 'n': idx = 0xf1; break;	case 'A': idx = 0xc3; break;
		    case 'O': idx = 0xd5; break;	case 'N': idx = 0xd1; break;
		}
	    }
	    else if (strcmp(buf + 1, "circ") == 0) {
		switch (*buf) {
		    case 'a': idx = 0xe2; break;	case 'e': idx = 0xea; break;
		    case 'i': idx = 0xee; break;	case 'o': idx = 0xf4; break;
		    case 'u': idx = 0xfb; break;	case 'A': idx = 0xc2; break;
		    case 'E': idx = 0xca; break;	case 'I': idx = 0xce; break;
		    case 'O': idx = 0xd4; break;	case 'U': idx = 0xdb; break;
		}
	    }
	    else if (strcmp(buf + 1, "uml") == 0) {
		switch (*buf) {
		    case 'a': idx = 0xe4; break;	case 'e': idx = 0xeb; break;
		    case 'i': idx = 0xef; break;	case 'o': idx = 0xf6; break;
		    case 'u': idx = 0xfc; break;	case 'A': idx = 0xc4; break;
		    case 'E': idx = 0xcb; break;	case 'I': idx = 0xfc; break;
		    case 'O': idx = 0xd6; break;	case 'U': idx = 0xdc; break;
		    case 'y': idx = 0xff; break;
		}
	    }
	    else if (strcmp(buf + 1, "cedil") == 0) {
		switch (*buf) {
		    case 'c': idx = 0xe7; break;	case 'C': idx = 0xc7; break;
		}
	    }
	    else if (strcmp(buf + 1, "slash") == 0) {
		switch (*buf) {
		    case 'o': idx = 0xf8; break;	case 'O': idx = 0xd8; break;
		}
	    }
	    else if (strcmp(buf + 1, "ring") == 0) {
		switch (*buf) {
		    case 'a': idx = 0xe5; break;	case 'A': idx = 0xc5; break;
		}
	    }
	    else if (strcasecmp(buf, "iexcl") == 0)
		idx = 0xa1;
	    else if (strcasecmp(buf, "cent") == 0)
		idx = 0xa2;
	    else if (strcasecmp(buf, "pound") == 0)
		idx = 0xa3;
	    else if (strcasecmp(buf, "curren") == 0)
		idx = 0xa4;
	    else if (strcasecmp(buf, "yen") == 0)
		idx = 0xa5;
	    else if (strcasecmp(buf, "brvbar") == 0)
		idx = 0xa6;
	    else if (strcasecmp(buf, "sect") == 0)
		idx = 0xa7;
	    else if (strcasecmp(buf, "uml") == 0)
		idx = 0xa8;
	    else if (strcasecmp(buf, "copy") == 0)
		idx = 0xa9;
	    else if (strcasecmp(buf, "ordf") == 0)
		idx = 0xaa;
	    else if (strcasecmp(buf, "laquo") == 0)
		idx = 0xab;
	    else if (strcasecmp(buf, "not") == 0)
		idx = 0xac;
	    else if (strcasecmp(buf, "shy") == 0)
		idx = 0xad;
	    else if (strcasecmp(buf, "reg") == 0)
		idx = 0xae;
	    else if (strcasecmp(buf, "macr") == 0)
		idx = 0xaf;
	    else if (strcasecmp(buf, "deg") == 0)
		idx = 0xb0;
	    else if (strcasecmp(buf, "plusmn") == 0)
		idx = 0xb1;
	    else if (strcasecmp(buf, "sup2") == 0)
		idx = 0xb2;
	    else if (strcasecmp(buf, "sup3") == 0)
		idx = 0xb3;
	    else if (strcasecmp(buf, "acute") == 0)
		idx = 0xb4;
	    else if (strcasecmp(buf, "micro") == 0)
		idx = 0xb5;
	    else if (strcasecmp(buf, "para") == 0)
		idx = 0xb6;
	    else if (strcasecmp(buf, "middot") == 0)
		idx = 0xb7;
	    else if (strcasecmp(buf, "cedil") == 0)
		idx = 0xb8;
	    else if (strcasecmp(buf, "supl") == 0)
		idx = 0xb9;
	    else if (strcasecmp(buf, "ordm") == 0)
		idx = 0xba;
	    else if (strcasecmp(buf, "raquo") == 0)
		idx = 0xbb;
	    else if (strcasecmp(buf, "frac14") == 0)
		idx = 0xbc;
	    else if (strcasecmp(buf, "frac12") == 0)
		idx = 0xbd;
	    else if (strcasecmp(buf, "frac34") == 0)
		idx = 0xbe;
	    else if (strcasecmp(buf, "iquest") == 0)
		idx = 0xbf;
	    else if (strcasecmp(buf, "AElig") == 0)
		idx = 0xc6;
	    else if (strcasecmp(buf, "ETH") == 0)
		idx = 0xd0;
	    else if (strcasecmp(buf, "THORN") == 0)
		idx = 0xde;
	    else if (strcasecmp(buf, "szlig") == 0)
		idx = 0xdf;
	    else if (strcasecmp(buf, "aelig") == 0)
		idx = 0xe6;
	    else if (strcasecmp(buf, "eth") == 0)
		idx = 0xf0;
	    else if (strcasecmp(buf, "thorn") == 0)
		idx = 0xfe;

	    parser->quark = Qentity;
	    if (entities[idx] == NULL) {
		entities[idx] = XtNew(XawTextBlock);
		entities[idx]->firstPos = 0;
		entities[idx]->length = 1;
		entities[idx]->ptr = chars + idx;
		entities[idx]->format = FMT8BIT;
		chars[idx] = idx;
	    }
	    parser->entity = entities[idx];
	    parser->end = parser->offset;
	    Html_Commit(parser);
	    parser->start = parser->end;
	}
    }
    /*NOTREACHED*/
}

/************************************************************************/
/* FORMAT								*/
/************************************************************************/
static int
Html_Put(Html_Parser *parser, int ch)
{
    if (ch != '\r') {
	if (parser->replace.length % 4096 == 0)
	    parser->replace.ptr = XtRealloc(parser->replace.ptr,
					    parser->replace.length + 4096);
	parser->replace.ptr[parser->replace.length++] = ch;
    }

    return (ch);
}

static void
Html_Puts(Html_Parser *parser, char *str)
{
    int len = strlen(str);

    if (parser->replace.length % 4096 == 0 ||
	parser->replace.length + len > parser->replace.length +
	(4096 - (parser->replace.length % 4096)))
	parser->replace.ptr = XtRealloc(parser->replace.ptr,
					parser->replace.length + 4096);
    memcpy(parser->replace.ptr + parser->replace.length, str, len);
    parser->replace.length += len;
}

static void
Html_FormatTag(Html_Parser *parser)
{
    int ch = 0, sz = 0;
    char buf[32];
    Html_TagInfo *info = NULL;

    switch (Html_Peek(parser)) {
	case '!':
	    Html_Put(parser, '<');
	    Html_Put(parser, Html_Get(parser));		/* eat `!' */
	    if (Html_Peek(parser) == '-') {
		/* comment */
		Html_Put(parser, Html_Get(parser));	/* eat `-' */
		if (Html_Peek(parser) == '-') {
		    int count = 0;

		    Html_Put(parser, Html_Get(parser));
		    while ((ch = Html_Peek(parser)) != EOF) {
			if (ch == '>' && count >= 2)
			    break;
			else if (ch == '-')
			    ++count;
			else
			    count = 0;
			Html_Put(parser, Html_Get(parser));
		    }
		    (void)Html_Get(parser);		/* eat `>' */
		    Html_Put(parser, '>');
		    return;
		}
	    }
	    break;
	case '?':
	    Html_Put(parser, '<');
	    break;
	case '/':
	    (void)Html_Get(parser);			/* eat `/' */
	    while (isalnum(Html_Peek(parser)) &&
		   ((sz + 1) < sizeof(buf)))
		buf[sz++] = ch = tolower(Html_Get(parser));
	    buf[sz] = '\0';
	    if ((info = Html_GetInfo(buf)) != NULL && info->adnl) {
		if (info->ident == Qpre && parser->pre) {
		    if (--parser->pre == 0)
			parser->column = 0;
		}
		parser->quark = Qetag;
		parser->spc = True;
		if (info->ident == Qp) {
		    while ((ch = Html_Peek(parser) != '>' && ch != EOF))
			(void)Html_Get(parser);
		    (void)Html_Get(parser);		/* eat '>' */
		    return;
		}
	    }
	    else if (info) {
		if (info->ident == Qol || info->ident == Qul) {
		    if (parser->list && --parser->list == 0 &&
			parser->desc == 0) {
			parser->quark = Qetag;
			Html_Put(parser, '\n');
			++parser->adnl;
			parser->column = 0;
		    }
		}
		else if (info->ident == Qdl) {
		    if (parser->desc && --parser->desc == 0 &&
			parser->list == 0) {
			parser->quark = Qetag;
			Html_Put(parser, '\n');
			++parser->adnl;
			parser->column = 0;
		    }
		}
	    }
	    Html_Puts(parser, "</");
	    Html_Puts(parser, buf);
	    break;
	default:
	    while (isalnum(Html_Peek(parser)) &&
		   ((sz + 1) < sizeof(buf)))
		buf[sz++] = tolower(Html_Get(parser));
	    buf[sz] = '\0';
	    if ((info = Html_GetInfo(buf)) != NULL && info->adnl) {
		if (info->ident == Qpre)
		    ++parser->pre;
		if (parser->quark != Qtag) {
		    if (parser->adnl < 2) {
			Html_Puts(parser, parser->adnl ? pnl : nlpnl);
			parser->adnl = 2;
			parser->spc = True;
			parser->column = 0;
		    }
		}
		parser->quark = Qtag;
		if (info->ident == Qp) {
		    while ((ch = Html_Peek(parser) != '>' && ch != EOF))
			(void)Html_Get(parser);
		    (void)Html_Get(parser);		/* eat '>' */
		    return;
		}
	    }
	    else if (info) {
		if (info->ident == Qol || info->ident == Qul) {
		    if (++parser->list == 1 && !parser->desc) {
			if (parser->adnl < 2) {
			    Html_Puts(parser, parser->adnl ? pnl : nlpnl);
			    parser->adnl = 2;
			    parser->column = 0;
			}
		    }
		    else if (parser->adnl == 0) {
			Html_Put(parser, '\n');
			parser->adnl = 1;
			parser->column = 0;
		    }
		    parser->spc = True;
		}
		else if (info->ident == Qli) {
		    if (parser->adnl == 0) {
			Html_Put(parser, '\n');
			parser->adnl = 1;
			parser->column = 0;
		    }
		}

		else if (info->ident == Qdl) {
		    if (++parser->desc == 1 && !parser->list) {
			if (parser->adnl < 2) {
			    Html_Puts(parser, parser->adnl ? pnl : nlpnl);
			    parser->adnl = 2;
			    parser->column = 0;
			}
		    }
		    else if (parser->adnl == 0) {
			Html_Put(parser, '\n');
			parser->adnl = 1;
			parser->column = 0;
		    }
		    parser->spc = True;
		}
		else if (info->ident == Qdd) {
		    if (parser->desc == 0) {
			if (parser->adnl < 2) {
			    Html_Puts(parser, parser->adnl ? pnl : nlpnl);
			    parser->adnl = 2;
			    parser->column = 0;
			}
		    }
		    else if (parser->adnl == 0) {
			Html_Put(parser, '\n');
			parser->adnl = 1;
			parser->column = 0;
		    }
		    parser->spc = True;
		}
		else if (info->ident == Qdt) {
		    if (parser->adnl == 0) {
			Html_Put(parser, '\n');
			parser->adnl = 1;
			parser->spc = True;
			parser->column = 0;
		    }
		}
	    }
	    Html_Put(parser, '<');
	    Html_Puts(parser, buf);
	    break;
    }

    sz = 0;
    while ((ch = Html_Peek(parser)) != '>' && ch != EOF) {
	if (isspace(ch)) {
	    (void)Html_Get(parser);
	    ++sz;
	    continue;
	}
	else if (sz) {
	    Html_Put(parser, ' ');
	    sz = 0;
	}
	Html_Put(parser, Html_Get(parser));
    }
    Html_Put(parser, Html_Get(parser));			/* eat `>' */
    if (info && info->ident == Qbr) {
	++parser->adnl;
	parser->spc = True;
	Html_Put(parser, '\n');
	parser->quark = info->ident;
	parser->column = 0;
    }
}

/* tags */
static int
Html_Format3(Html_Parser *parser)
{
    int ch;

    for (;;) {
	if ((ch = Html_Get(parser)) == '<') {
	    if (parser->quark == Qspace && parser->spc == False) {
		Html_Put(parser, ' ');
		parser->spc = True;
	    }

/*	    parser->quark = Qhide;*/
	    Html_FormatTag(parser);
	}
	else
	    return (ch);
    }
    /*NOTREACHED*/
}

/* entities */
static int
Html_Format2(Html_Parser *parser)
{
    int ch;

    for (ch = Html_Format3(parser); ch == '&'; ch = Html_Format3(parser)) {
	Html_Put(parser, '&');
	while ((ch = Html_Peek(parser)) != ';') {
	    if (isspace(ch) || ch == EOF)
		break;
	    Html_Put(parser, Html_Get(parser));
	}
	if (ch != EOF)
	    Html_Put(parser, Html_Get(parser));
	else
	    break;
	if (parser->pre)
	    ++parser->column;
    }

    return (ch);
}

/* spaces */
static int
Html_Format1(Html_Parser *parser)
{
    int ch;

    for (;;) {
	if ((ch = Html_Format2(parser)) == EOF)
	    return (ch);

	if (parser->quark == Qetag) {
	    if (parser->adnl < 2) {
		Html_Puts(parser, parser->adnl ? pnl : nlpnl);
		parser->adnl = 2;
		parser->spc = True;
	    }
	}
	else if (parser->quark == Qspace && parser->spc == False) {
	    Html_Put(parser, ' ');
	    parser->spc = True;
	}

	if (!parser->pre && isspace(ch))
	    parser->quark = Qspace;
	else {
	    if (parser->pre) {
		if (parser->spc) {
		    /* did not yet see any non space character */
		    if (isspace(ch)) {
			if (ch == '\n') {
			    parser->column = 0;
			    parser->spc = False;
			    parser->adnl = 1;
			}
			else if (ch == '\t')
			    parser->column += 8 - (parser->column % 8);
			else
			    ++parser->column;
			continue;
		    }
		    else {
			int column = parser->column;

			while (column-- > 0)
			    Html_Put(parser, ' ');
			parser->spc = False;
			parser->adnl = 0;
		    }
		}
		else if (ch == '\n') {
		    ++parser->adnl;
		    parser->column = 0;
		}
		else if (ch == '\t') {
		    int column = parser->column + (8 - (parser->column % 8));

		    parser->adnl = 0;
		    while (parser->column < column) {
			Html_Put(parser, ' ');
			++parser->column;
		    }
		    continue;
		}
		else {
		    parser->adnl = 0;
		    ++parser->column;
		}
	    }
	    else
		parser->adnl = 0;
	    Html_Put(parser, ch);
	    parser->quark = Qdefault;
	    parser->spc = False;
	}
    }
}

/************************************************************************/
/* ARGUMENTS								*/
/************************************************************************/
static void
Html_AArgs(Html_Parser *parser, Html_Item *item)
{
    int ch, sz;
    char buf[32];

    /*CONSTCOND*/
    while (True) {
	sz = 0;
	while ((ch = Html_Peek(parser)) != '>' && ch != EOF) {
	    if (isalnum(ch))
		break;
	    else
		(void)Html_Get(parser);
	}

	if (ch == '>' || ch == EOF)
	    return;
	buf[sz++] = tolower(Html_Get(parser));
	while ((ch = Html_Peek(parser)) != '>' && ch != EOF)
	    if (isalnum(ch))
		buf[sz++] = tolower(Html_Get(parser));
	    else
		break;
	buf[sz] = '\0';
	if (strcmp(buf, "href") == 0) {
	    item->combine = XawTextSinkCopyProperty(XawTextGetSink(text),
						    item->info->ident);
	    item->override = True;
	    item->combine->xlfd_mask = 0L;
	    item->combine->mask = XAW_TPROP_UNDERLINE | XAW_TPROP_FOREGROUND;
	    item->combine->foreground = parser->alink;
	    return;
	}
	while ((ch = Html_Peek(parser)) != '>' && ch != EOF) {
	    if (isspace(ch))
		break;
	    else
		(void)Html_Get(parser);
	}
    }
}

static void
Html_FontArgs(Html_Parser *parser, Html_Item *item)
{
    int ch, sz;
    char name[32], value[256], xlfd[128];

    item->combine = XawTextSinkCopyProperty(XawTextGetSink(text),
					    Qdefault);
    item->override = True;
    item->combine->mask = item->combine->xlfd_mask = 0L;

    /*CONSTCOND*/
    while (True) {
	/* skip white spaces */
	while ((ch = Html_Peek(parser)) != '>' && ch != EOF) {
	    if (isalnum(ch))
		break;
	    else
		(void)Html_Get(parser);
	}

	if (ch == '>' || ch == EOF)
	    return;

	/* read option name */
	sz = 0;
	name[sz++] = tolower(Html_Get(parser));
	while ((ch = Html_Peek(parser)) != '>' && ch != EOF)
	    if (isalnum(ch) && (sz + 1 < sizeof(name)))
		name[sz++] = tolower(Html_Get(parser));
	    else
		break;
	name[sz] = '\0';

	if (ch != '=')
	    continue;
	(void)Html_Get(parser);	/* skip `=' */
	if (Html_Peek(parser) == '"')
	    (void)Html_Get(parser);

	sz = 0;
	while ((ch = Html_Peek(parser)) != '>' && ch != EOF) {
	    if (!isspace(ch) && (sz + 1 < sizeof(value)))
		value[sz++] = Html_Get(parser);
	    else
		break;
	}
	value[sz] = '\0';
	if (sz > 0 && value[sz - 1] == '"')
	    value[--sz] = '\0';

	if (strcmp(name, "color") == 0) {
	    XColor  color, exact;

	    if (XAllocNamedColor(XtDisplay(toplevel), toplevel->core.colormap,
				 value, &color, &exact)) {
		item->combine->mask |= XAW_TPROP_FOREGROUND;
		item->combine->foreground = color.pixel;
	    }
	}
	else if (strcmp(name, "face") == 0) {
	    int count = 0;
	    char *ptr, *family, **font_list;

	    ptr = value;
	    do {
		family = ptr;
		ptr = strchr(ptr, ',');
		if (ptr)
		    *ptr++ = '\0';
		XmuSnprintf(xlfd, sizeof(xlfd), "-*-%s-*-*-*-*-*-*-*-*-*-*-*-*",
			    family);
		font_list = XListFonts(XtDisplay(toplevel), xlfd, 1, &count);
		if (font_list)
		    XFreeFontNames(font_list);
		if (count)
		    break;
	    } while (ptr);
	    if (count) {
		item->combine->xlfd_mask |= XAW_TPROP_FAMILY;
		item->combine->family = XrmStringToQuark(family);
	    }
	}
	else if (strcmp(name, "size") == 0) {
	    int size, sign = 0;

	    if (isalnum(*value)) {
		size = atoi(value);
		sign = 0;
	    }
	    else {
		char *str = XrmQuarkToString(item->combine->pixel_size);

		size = str ? atoi(str) : 12;
		if (*value == '+') {
		    size += atoi(value + 1);
		    sign = 1;
		}
		else if (*value == '-') {
		    size -= atoi(value + 1);
		    sign = -1;
		}
	    }

	    if (item->combine->xlfd != NULLQUARK) {
		int count, ucount, dcount, usize, dsize;
		char **current, **result, **up, **down;

		current = result = up = down = NULL;
		/* try to load an appropriate font */
		XmuSnprintf(value, sizeof(value),
			    "-*-%s-%s-%s-*--%%d-*-*-*-*-*-%s-%s",
			    XrmQuarkToString(item->combine->family),
			    XrmQuarkToString(item->combine->weight),
			    XrmQuarkToString(item->combine->slant),
			    XrmQuarkToString(item->combine->registry),
			    XrmQuarkToString(item->combine->encoding));
		XmuSnprintf(xlfd, sizeof(xlfd), value,
			    atoi(XrmQuarkToString(item->combine->pixel_size)));
		current = XListFonts(XtDisplay(toplevel), xlfd, 1, &count);
		if (count) {
		    ucount = dcount = usize = dsize = 0;

		    XmuSnprintf(xlfd, sizeof(xlfd), value, size);
		    result = XListFonts(XtDisplay(toplevel), xlfd, 1, &count);
		    if (count == 0 || strstr(*result, "-0-")) {
			if (sign <= 0) {
			    sz = dsize = size;
			    while (dcount == 0 && --sz > size - 8 && sz > 1) {
				XmuSnprintf(xlfd, sizeof(xlfd), value, sz);
				down = XListFonts(XtDisplay(toplevel), xlfd,
						  1, &dcount);
				if (dcount && strstr(*down, "-0-") != NULL) {
				    XFreeFontNames(down);
				    down = NULL;
				    dcount = 0;
				}
			    }
			    if (dcount)
				dsize = sz;
			}
			if (sign >= 0) {
			    sz = usize = size;
			    while (ucount == 0 && ++sz < size + 8) {
				XmuSnprintf(xlfd, sizeof(xlfd), value, sz);
				up = XListFonts(XtDisplay(toplevel), xlfd,
						1, &ucount);
				if (ucount && strstr(*up, "-0-") != NULL) {
				    XFreeFontNames(up);
				    up = NULL;
				    ucount = 0;
				}
			    }
			    if (ucount)
				usize = sz;
			}
			if (ucount && dcount)
			    size = size - dsize < usize - size ? dsize : usize;
			else if (ucount)
			    size = usize;
			else if (dcount)
			    size = dsize;
		    }
		    if (current)
			XFreeFontNames(current);
		    if (result)
			XFreeFontNames(result);
		    if (up)
			XFreeFontNames(up);
		    if (down)
			XFreeFontNames(down);
		}
	    }

	    XmuSnprintf(value, sizeof(value), "%d", size);
	    item->combine->xlfd_mask |= XAW_TPROP_PIXELSIZE;
	    item->combine->pixel_size = XrmStringToQuark(value);
	}

	while ((ch = Html_Peek(parser)) != '>' && ch != EOF) {
	    if (isspace(ch))
		break;
	    else
		(void)Html_Get(parser);
	}
    }
}
