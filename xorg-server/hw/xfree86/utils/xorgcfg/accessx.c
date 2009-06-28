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
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <X11/XKBlib.h>
#include <X11/Shell.h>
#include <X11/Xaw/Command.h>
#include <X11/Xaw/Form.h>
#include <X11/Xaw/Label.h>
#include <X11/Xaw/Scrollbar.h>
#include <X11/Xaw/Toggle.h>
#include "keyboard-cfg.h"

#define MAX_TIMEOUT		20
#define MAX_MOUSE_SPEED		500
#define MAX_MOUSE_TIME		4
#define MAX_MOUSE_DELAY		2.09
#define MAX_REPEAT_RATE		8.04
#define MAX_REPEAT_DELAY	6.04
#define MAX_SLOW_TIME		4
#define MAX_BOUNCE_TIME		4

/*
 * Types
 */
typedef struct {
    Widget label, number, scroller;
    double min, max, value, resolution;
    Bool integer;
} Scale;

/*
 * Initialization
 */
static Widget shell, accessx, enable, timeoutToggle, form, apply;
static Widget sticky, stickyAuto, stickyBeep;
static Widget mouse;
static Widget repeat;
static Widget slowToggle, slowPressed, slowAccepted;
static Widget bounceToggle;
static Scale *timeout, *mouseSpeed, *mouseTime, *mouseDelay, *slow,
	*repeatRate, *repeatDelay, *bounce;
extern Widget work;

/*
 * Prototypes
 */
static void CreateAccessXHelpDialog(void);
static void EnableCallback(Widget, XtPointer, XtPointer);
static void ScaleEnableCallback(Widget, XtPointer, XtPointer);
static void ScaleJumpCallback(Widget, XtPointer, XtPointer);

static void ApplyCallback(Widget, XtPointer, XtPointer);
static void AccessXInitialize(void);

void CloseAccessXAction(Widget, XEvent*, String*, Cardinal*);
void AccessXConfigureStart(void);
void AccessXConfigureEnd(void);

/*
 * Implementation
 */
void
startaccessx(void)
{
    InitializeKeyboard();

    if (xkb_info->xkb) {
	XkbGetControls(DPY, XkbAllControlsMask, xkb_info->xkb);
	if (xkb_info->xkb->ctrls == NULL)
	    xkb_info->xkb->ctrls = (XkbControlsPtr)
		XtCalloc(1, sizeof(XkbControlsRec));

	xkb_info->xkb->ctrls->enabled_ctrls |= XkbMouseKeysMask |
					       XkbMouseKeysAccelMask;
	xkb_info->xkb->ctrls->mk_delay = 40;
	xkb_info->xkb->ctrls->mk_interval = 10;
	xkb_info->xkb->ctrls->mk_time_to_max = 1000;
	xkb_info->xkb->ctrls->mk_max_speed = 500;
	xkb_info->xkb->ctrls->mk_curve = 0;
	XkbSetControls(DPY, XkbAllControlsMask, xkb_info->xkb);
	(void)UpdateKeyboard(True);
	CreateAccessXHelpDialog();
    }
}

void
CreateAccessXHelpDialog()
{
    Widget form;

    shell = XtVaCreatePopupShell("accessx", transientShellWidgetClass, toplevel,
				 XtNx, toplevel->core.x + toplevel->core.width,
				 XtNy, toplevel->core.y, NULL);
    form = XtCreateManagedWidget("form", formWidgetClass, shell, NULL, 0);
    XtCreateManagedWidget("label", labelWidgetClass, form, NULL, 0);
    XtCreateManagedWidget("lock", labelWidgetClass, form, NULL, 0);
    XtCreateManagedWidget("div", labelWidgetClass, form, NULL, 0);
    XtCreateManagedWidget("mul", labelWidgetClass, form, NULL, 0);
    XtCreateManagedWidget("minus", labelWidgetClass, form, NULL, 0);
    XtCreateManagedWidget("7", labelWidgetClass, form, NULL, 0);
    XtCreateManagedWidget("8", labelWidgetClass, form, NULL, 0);
    XtCreateManagedWidget("9", labelWidgetClass, form, NULL, 0);
    XtCreateManagedWidget("plus", labelWidgetClass, form, NULL, 0);
    XtCreateManagedWidget("4", labelWidgetClass, form, NULL, 0);
    XtCreateManagedWidget("5", labelWidgetClass, form, NULL, 0);
    XtCreateManagedWidget("6", labelWidgetClass, form, NULL, 0);
    XtCreateManagedWidget("1", labelWidgetClass, form, NULL, 0);
    XtCreateManagedWidget("2", labelWidgetClass, form, NULL, 0);
    XtCreateManagedWidget("3", labelWidgetClass, form, NULL, 0);
    XtCreateManagedWidget("enter", labelWidgetClass, form, NULL, 0);
    XtCreateManagedWidget("0", labelWidgetClass, form, NULL, 0);
    XtCreateManagedWidget("del", labelWidgetClass, form, NULL, 0);

    XtRealizeWidget(shell);
    XSetWMProtocols(DPY, XtWindow(shell), &wm_delete_window, 1);
    XtPopup(shell, XtGrabNone);
}

/*ARGSUSED*/
void
CloseAccessXAction(Widget w, XEvent *event, String *params, Cardinal *num_params)
{
    XtPopdown(shell);
}

static void
AccessXInitialize(void)
{
    static int first = 1;
    Arg args[1];
    Boolean state;
    Widget stickyForm, mouseForm, repeatForm, slowForm, bounceForm;
    float val, tmp;

    if (!first)
	return;
    first = 0;

    InitializeKeyboard();

    XkbGetControls(DPY, XkbAllControlsMask, xkb_info->xkb);
    if (xkb_info->xkb->ctrls == NULL)
	xkb_info->xkb->ctrls = (XkbControlsPtr)
	    XtCalloc(1, sizeof(XkbControlsRec));

    timeout = XtNew(Scale);
    accessx = XtCreateWidget("accessxForm", formWidgetClass, work, NULL, 0);
    enable = XtVaCreateManagedWidget("enable", toggleWidgetClass, accessx,
				     XtNstate,
				     (xkb_info->xkb->ctrls->enabled_ctrls &
				      (XkbAccessXKeysMask | XkbStickyKeysMask |
				       XkbSlowKeysMask | XkbBounceKeysMask)) != 0, NULL);

    apply = XtCreateManagedWidget("apply", commandWidgetClass, accessx, NULL, 0);
    XtAddCallback(apply, XtNcallback, ApplyCallback, NULL);

    form = XtCreateManagedWidget("Accessx", formWidgetClass, accessx, NULL, 0);
    timeoutToggle = XtVaCreateManagedWidget("timeoutToggle", toggleWidgetClass,
					    form, XtNstate,
					    xkb_info->xkb->ctrls->ax_timeout > 60
					    && xkb_info->xkb->ctrls->ax_timeout
					    < 30000, NULL);
    XtAddCallback(timeoutToggle, XtNcallback, ScaleEnableCallback,
		  (XtPointer)timeout);
    timeout->label = XtCreateManagedWidget("timeoutLabel", labelWidgetClass,
					   form, NULL, 0);
    timeout->number = XtCreateManagedWidget("timeoutNumber", labelWidgetClass,
					    form, NULL, 0);
    timeout->scroller = XtCreateManagedWidget("timeoutScroller",
					      scrollbarWidgetClass,
					      form, NULL, 0);
    XtAddCallback(timeout->scroller, XtNjumpProc, ScaleJumpCallback,
		  (XtPointer)timeout);
    timeout->min = 1;
    timeout->max = MAX_TIMEOUT;
    timeout->resolution = 1;
    timeout->integer = True;

    sticky = XtVaCreateManagedWidget("sticky", toggleWidgetClass, form,
				     XtNstate,
				     (xkb_info->xkb->ctrls->enabled_ctrls &
				      XkbStickyKeysMask) != 0, NULL);
    stickyForm = XtCreateManagedWidget("stickyForm", formWidgetClass,
				       form, NULL, 0);
    XtAddCallback(sticky, XtNcallback, EnableCallback, (XtPointer)stickyForm);
    stickyAuto = XtVaCreateManagedWidget("auto", toggleWidgetClass, stickyForm,
					 XtNstate,
					 (xkb_info->xkb->ctrls->ax_options &
					  XkbAX_LatchToLockMask) == 0, NULL);
    stickyBeep = XtVaCreateManagedWidget("beep", toggleWidgetClass, stickyForm,
					 XtNstate,
					 (xkb_info->xkb->ctrls->ax_options &
					  XkbAX_StickyKeysFBMask) != 0, NULL);

    mouse = XtVaCreateManagedWidget("mouseKeys", toggleWidgetClass, form,
				    XtNstate,
				    (xkb_info->xkb->ctrls->enabled_ctrls &
				     (XkbMouseKeysMask | XkbMouseKeysAccelMask))
				    != 0, NULL);
    mouseForm = XtCreateManagedWidget("mouseForm", formWidgetClass,
				      form, NULL, 0);
    XtAddCallback(mouse, XtNcallback, EnableCallback, (XtPointer)mouseForm);
    mouseSpeed = XtNew(Scale);
    mouseSpeed->label = XtCreateManagedWidget("speedLabel", labelWidgetClass,
					      mouseForm, NULL, 0);
    mouseSpeed->number = XtCreateManagedWidget("speedNumber", labelWidgetClass,
					      mouseForm, NULL, 0);
    mouseSpeed->scroller = XtCreateManagedWidget("speedScroller",
						 scrollbarWidgetClass,
						 mouseForm, NULL, 0);
    XtAddCallback(mouseSpeed->scroller, XtNjumpProc, ScaleJumpCallback,
		  (XtPointer)mouseSpeed);
    mouseSpeed->min = 10;
    mouseSpeed->max = MAX_MOUSE_SPEED;
    mouseSpeed->resolution = 10;
    mouseSpeed->integer = True;
    mouseTime = XtNew(Scale);
    mouseTime->label = XtCreateManagedWidget("timeLabel", labelWidgetClass,
					     mouseForm, NULL, 0);
    mouseTime->number = XtCreateManagedWidget("timeNumber", labelWidgetClass,
					      mouseForm, NULL, 0);
    mouseTime->scroller = XtCreateManagedWidget("timeScroller",
						scrollbarWidgetClass,
						mouseForm, NULL, 0);
    XtAddCallback(mouseTime->scroller, XtNjumpProc, ScaleJumpCallback,
		  (XtPointer)mouseTime);
    mouseTime->min = .1;
    mouseTime->max = MAX_MOUSE_TIME;
    mouseTime->resolution = .1;
    mouseTime->integer = False;
    mouseDelay = XtNew(Scale);
    mouseDelay->label = XtCreateManagedWidget("delayLabel", labelWidgetClass,
					      mouseForm, NULL, 0);
    mouseDelay->number = XtCreateManagedWidget("delayNumber", labelWidgetClass,
					       mouseForm, NULL, 0);
    mouseDelay->scroller = XtCreateManagedWidget("delayScroller",
						 scrollbarWidgetClass,
						 mouseForm, NULL, 0);
    XtAddCallback(mouseDelay->scroller, XtNjumpProc, ScaleJumpCallback,
		  (XtPointer)mouseDelay);
    mouseDelay->min = .1;
    mouseDelay->max = MAX_MOUSE_DELAY;
    mouseDelay->resolution = .1;
    mouseDelay->integer = False;

    repeat = XtVaCreateManagedWidget("repeatKeys", toggleWidgetClass, form,
				     XtNstate,
				    (xkb_info->xkb->ctrls->enabled_ctrls &
				     XkbRepeatKeysMask) != 0, NULL);
    repeatForm = XtCreateManagedWidget("repeatForm", formWidgetClass,
				       form, NULL, 0);
    XtAddCallback(repeat, XtNcallback, EnableCallback, (XtPointer)repeatForm);
    repeatRate = XtNew(Scale);
    repeatRate->label = XtCreateManagedWidget("rateLabel", labelWidgetClass,
					      repeatForm, NULL, 0);
    repeatRate->number = XtCreateManagedWidget("rateNumber", labelWidgetClass,
					       repeatForm, NULL, 0);
    repeatRate->scroller = XtCreateManagedWidget("rateScroller",
						 scrollbarWidgetClass,
						 repeatForm, NULL, 0);
    XtAddCallback(repeatRate->scroller, XtNjumpProc, ScaleJumpCallback,
		  (XtPointer)repeatRate);
    repeatRate->min = .05;
    repeatRate->max = MAX_REPEAT_RATE;
    repeatRate->resolution = .05;
    repeatRate->integer = False;
    repeatDelay = XtNew(Scale);
    repeatDelay->label = XtCreateManagedWidget("delayLabel", labelWidgetClass,
					      repeatForm, NULL, 0);
    repeatDelay->number = XtCreateManagedWidget("delayNumber", labelWidgetClass,
					       repeatForm, NULL, 0);
    repeatDelay->scroller = XtCreateManagedWidget("delayScroller",
						 scrollbarWidgetClass,
						 repeatForm, NULL, 0);
    XtAddCallback(repeatDelay->scroller, XtNjumpProc, ScaleJumpCallback,
		  (XtPointer)repeatDelay);
    repeatDelay->min = .05;
    repeatDelay->max = MAX_REPEAT_DELAY;
    repeatDelay->resolution = .05;
    repeatDelay->integer = False;

    slowToggle = XtVaCreateManagedWidget("slow", toggleWidgetClass,
					 form, XtNstate,
					 (xkb_info->xkb->ctrls->enabled_ctrls &
					 XkbSlowKeysMask) != 0, NULL);
    slowForm = XtCreateManagedWidget("slowForm", formWidgetClass,
				     form, NULL, 0);
    XtAddCallback(slowToggle, XtNcallback, EnableCallback, (XtPointer)slowForm);
    XtCreateManagedWidget("beep", labelWidgetClass, slowForm, NULL, 0);
    slowPressed = XtVaCreateManagedWidget("pressed", toggleWidgetClass,
					  slowForm, XtNstate,
					  (xkb_info->xkb->ctrls->ax_options &
					  XkbAX_SKPressFBMask) != 0,
					  NULL);
    slowAccepted = XtVaCreateManagedWidget("accepted", toggleWidgetClass,
					   slowForm, XtNstate,
					   (xkb_info->xkb->ctrls->ax_options &
					   XkbAX_SKAcceptFBMask) != 0,
					   NULL);
    slow = XtNew(Scale);
    slow->label = XtCreateManagedWidget("slowLabel", labelWidgetClass,
					slowForm, NULL, 0);
    slow->number = XtCreateManagedWidget("slowNumber", labelWidgetClass,
					  slowForm, NULL, 0);
    slow->scroller = XtCreateManagedWidget("slowScroller",
					   scrollbarWidgetClass,
					   slowForm, NULL, 0);
    XtAddCallback(slow->scroller, XtNjumpProc, ScaleJumpCallback,
		  (XtPointer)slow);
    slow->min = 0.1;
    slow->max = MAX_SLOW_TIME;
    slow->resolution = 0.1;
    slow->integer = False;

    bounceToggle = XtVaCreateManagedWidget("bounce", toggleWidgetClass,
					   form, XtNstate,
					   (xkb_info->xkb->ctrls->enabled_ctrls &
					   XkbBounceKeysMask) != 0,
					   NULL);
    bounceForm = XtCreateManagedWidget("bounceForm", formWidgetClass,
				     form, NULL, 0);
    XtAddCallback(bounceToggle, XtNcallback, EnableCallback, (XtPointer)bounceForm);
    bounce = XtNew(Scale);
    bounce->label = XtCreateManagedWidget("bounceLabel", labelWidgetClass,
					bounceForm, NULL, 0);
    bounce->number = XtCreateManagedWidget("bounceNumber", labelWidgetClass,
					  bounceForm, NULL, 0);
    bounce->scroller = XtCreateManagedWidget("bounceScroller",
					   scrollbarWidgetClass,
					   bounceForm, NULL, 0);
    XtAddCallback(bounce->scroller, XtNjumpProc, ScaleJumpCallback,
		  (XtPointer)bounce);
    bounce->min = 0.1;
    bounce->max = MAX_BOUNCE_TIME;
    bounce->resolution = 0.1;
    bounce->integer = False;

    XtRealizeWidget(accessx);

    XtSetArg(args[0], XtNstate, &state);
    XtGetValues(timeoutToggle, args, 1);
    ScaleEnableCallback(enable, (XtPointer)timeout, (XtPointer)(long)state);
    if (xkb_info->xkb->ctrls->ax_timeout > 60)
	val = (float)(xkb_info->xkb->ctrls->ax_timeout - 60) /
	      (float)(MAX_TIMEOUT * 60);
    else
	val = 0;
    ScaleJumpCallback(timeout->scroller, (XtPointer)timeout, (XtPointer)&val);

    XtSetArg(args[0], XtNstate, &state);
    XtGetValues(sticky, args, 1);
    EnableCallback(sticky, (XtPointer)stickyForm, (XtPointer)(long)state);

    XtSetArg(args[0], XtNstate, &state);
    XtGetValues(mouse, args, 1);
    EnableCallback(mouse, (XtPointer)mouseForm, (XtPointer)(long)state);
    if (xkb_info->xkb->ctrls->mk_time_to_max > 10)
	val = (float)((xkb_info->xkb->ctrls->mk_time_to_max * (40. / 10.))) /
	      (float)(MAX_MOUSE_TIME * 100);
    else
	val = 10.0 / (float)(MAX_MOUSE_TIME * 100);
    ScaleJumpCallback(mouseTime->scroller, (XtPointer)mouseTime,
		      (XtPointer)&val);
    tmp = mouseTime->value;
    if (xkb_info->xkb->ctrls->mk_max_speed != 0)
	val = (float)(xkb_info->xkb->ctrls->mk_max_speed / tmp - 10) /
	      (float)MAX_MOUSE_SPEED;
    else
	val = 10.0 / (float)MAX_MOUSE_SPEED;
    ScaleJumpCallback(mouseSpeed->scroller, (XtPointer)mouseSpeed,
		      (XtPointer)&val);
    if (xkb_info->xkb->ctrls->mk_delay > 10)
	val = (float)(xkb_info->xkb->ctrls->mk_delay - 10) /
	      (float)(MAX_MOUSE_DELAY * 100);
    else
	val = 10.0 / (float)(MAX_MOUSE_DELAY * 100);
    ScaleJumpCallback(mouseDelay->scroller, (XtPointer)mouseDelay,
		      (XtPointer)&val);

    XtSetArg(args[0], XtNstate, &state);
    XtGetValues(repeat, args, 1);
    EnableCallback(repeat, (XtPointer)repeatForm, (XtPointer)(long)state);
    if (xkb_info->xkb->ctrls->repeat_interval > 5)
	val = (float)(xkb_info->xkb->ctrls->repeat_interval - 5) /
	      (float)(MAX_REPEAT_RATE * 1000);
    else
	val = 5.0 / (float)(MAX_REPEAT_RATE * 1000);
    ScaleJumpCallback(repeatRate->scroller, (XtPointer)repeatRate,
		      (XtPointer)&val);
    if (xkb_info->xkb->ctrls->repeat_delay > 5)
	val = (float)(xkb_info->xkb->ctrls->repeat_delay - 5) /
	      (float)(MAX_REPEAT_DELAY * 1000);
    else
	val = 5.0 / (float)(MAX_REPEAT_DELAY * 1000);
    ScaleJumpCallback(repeatDelay->scroller, (XtPointer)repeatDelay,
		      (XtPointer)&val);

    XtSetArg(args[0], XtNstate, &state);
    XtGetValues(slowToggle, args, 1);
    EnableCallback(slowToggle, (XtPointer)slowForm, (XtPointer)(long)state);
    if (xkb_info->xkb->ctrls->slow_keys_delay > 10)
	val = (float)(xkb_info->xkb->ctrls->repeat_delay - 10) /
	      (float)(MAX_SLOW_TIME * 1000);
    else
	val = 10.0 / (float)(MAX_SLOW_TIME * 1000);
    ScaleJumpCallback(slow->scroller, (XtPointer)slow, (XtPointer)&val);

    XtSetArg(args[0], XtNstate, &state);
    XtGetValues(bounceToggle, args, 1);
    EnableCallback(bounceToggle, (XtPointer)bounceForm, (XtPointer)(long)state);
    if (xkb_info->xkb->ctrls->debounce_delay > 10)
	val = (float)(xkb_info->xkb->ctrls->debounce_delay - 10) /
	      (float)(MAX_BOUNCE_TIME * 1000);
    else
	val = 10.0 / (float)(MAX_BOUNCE_TIME * 1000);
    ScaleJumpCallback(bounce->scroller, (XtPointer)bounce, (XtPointer)&val);

    XtSetArg(args[0], XtNstate, &state);
    XtGetValues(enable, args, 1);
}

void
AccessXConfigureStart(void)
{
    AccessXInitialize();

    XtMapWidget(accessx);
}

void
AccessXConfigureEnd(void)
{
    XtUnmapWidget(accessx);
}

/*ARGSUSED*/
static void
EnableCallback(Widget w, XtPointer user_data, XtPointer call_data)
{
    XtSetSensitive((Widget)user_data, (long)call_data);
}

/*ARGSUSED*/
static void
ScaleEnableCallback(Widget w, XtPointer user_data, XtPointer call_data)
{
    Scale *scale = (Scale*)user_data;

    XtSetSensitive(scale->label, (long)call_data);
    XtSetSensitive(scale->number, (long)call_data);
    XtSetSensitive(scale->scroller, (long)call_data);
}

static void
ScaleJumpCallback(Widget w, XtPointer user_data, XtPointer call_data)
{
    Scale *scale = (Scale*)user_data;
    float percent = *(float *)call_data, timeout = percent * scale->max;
    int x;
    char str[8];
    Arg args[1];

    if (timeout >= scale->max - scale->min)
	timeout = scale->max - scale->min;

    if (scale->integer) {
	int tm = timeout + scale->min;

	tm -= tm % (int)scale->resolution;
	XmuSnprintf(str, sizeof(str), "%i", tm);
	scale->value = tm;
    }
    else {
	long tm = (timeout + scale->min) * 1e+6;

	tm -= tm % (long)(scale->resolution * 1e+6);
	scale->value = (double)tm / 1e+6;
	XmuSnprintf(str, sizeof(str), "%f", scale->value);
    }

    XtSetArg(args[0], XtNlabel, str);
    XtSetValues(scale->number, args, 1);
    x = w->core.x + w->core.border_width;
    x += ((double)(w->core.width - scale->number->core.width) / scale->max) * timeout;
    XtMoveWidget(scale->number, x, scale->number->core.y);
    XawScrollbarSetThumb(w, timeout / (scale->max - scale->min),
			 scale->resolution / (scale->max - scale->min));
}

/*ARGSUSED*/
static void
ApplyCallback(Widget w, XtPointer user_data, XtPointer call_data)
{
    Arg args[1];
    Boolean state;

    XkbGetControls(DPY, XkbAllControlsMask, xkb_info->xkb);

    /* Enable AccessX */
    XtSetArg(args[0], XtNstate, &state);
    XtGetValues(enable, args, 1);
    if (state) {
	xkb_info->config.initial_ctrls |= XkbAccessXKeysMask;
	xkb_info->xkb->ctrls->enabled_ctrls |= XkbAccessXKeysMask;
    }
    else {
	xkb_info->config.initial_ctrls &= ~XkbAccessXKeysMask;
	xkb_info->xkb->ctrls->enabled_ctrls &= ~XkbAccessXKeysMask;
    }

    /* Timeout */
    XtSetArg(args[0], XtNstate, &state);
    XtGetValues(timeoutToggle, args, 1);
    if (state)
	xkb_info->config.ax_timeout =
	xkb_info->xkb->ctrls->ax_timeout = timeout->value * 60;
    else
	xkb_info->config.ax_timeout =
	xkb_info->xkb->ctrls->ax_timeout = 65535;

    /* Enable StickyKeys */
    XtSetArg(args[0], XtNstate, &state);
    XtGetValues(sticky, args, 1);
    if (state) {
	xkb_info->config.initial_ctrls |= XkbStickyKeysMask;
	xkb_info->xkb->ctrls->enabled_ctrls |= XkbStickyKeysMask;
    }
    else {
	xkb_info->config.initial_ctrls &= ~XkbStickyKeysMask;
	xkb_info->xkb->ctrls->enabled_ctrls &= ~XkbStickyKeysMask;
    }
    XtSetArg(args[0], XtNstate, &state);
    XtGetValues(stickyAuto, args, 1);
    if (state) {
	xkb_info->config.initial_opts &= ~XkbAX_TwoKeysMask;
	xkb_info->config.initial_opts &= ~XkbAX_LatchToLockMask;
	xkb_info->xkb->ctrls->ax_options &= ~XkbAX_TwoKeysMask;
	xkb_info->xkb->ctrls->ax_options &= ~XkbAX_LatchToLockMask;
    }
    else {
	xkb_info->config.initial_opts &= ~XkbAX_TwoKeysMask;
	xkb_info->config.initial_opts |= XkbAX_LatchToLockMask;
	xkb_info->xkb->ctrls->ax_options &= ~XkbAX_TwoKeysMask;
	xkb_info->xkb->ctrls->ax_options |= XkbAX_LatchToLockMask;
    }
    XtSetArg(args[0], XtNstate, &state);
    XtGetValues(stickyBeep, args, 1);
    if (state) {
	xkb_info->config.initial_opts |= XkbAX_StickyKeysFBMask;
	xkb_info->xkb->ctrls->ax_options |= XkbAX_StickyKeysFBMask;
    }
    else {
	xkb_info->config.initial_opts &= ~XkbAX_StickyKeysFBMask;
	xkb_info->xkb->ctrls->ax_options &= ~XkbAX_StickyKeysFBMask;
    }

    /* Enable MouseKeys */
    XtSetArg(args[0], XtNstate, &state);
    XtGetValues(mouse, args, 1);
    if (state) {
	xkb_info->config.initial_ctrls |= XkbMouseKeysAccelMask;
	xkb_info->xkb->ctrls->enabled_ctrls |= XkbMouseKeysMask |
					       XkbMouseKeysAccelMask;
	xkb_info->config.mk_delay =
	    xkb_info->xkb->ctrls->mk_delay = mouseDelay->value * 100;
	xkb_info->config.mk_interval =
	    xkb_info->xkb->ctrls->mk_interval = 40;
	xkb_info->config.mk_time_to_max =
	xkb_info->xkb->ctrls->mk_time_to_max =
	    (mouseTime->value * 1000) / xkb_info->xkb->ctrls->mk_interval;
	xkb_info->config.mk_max_speed =
	xkb_info->xkb->ctrls->mk_max_speed =
	    mouseSpeed->value * mouseTime->value;
	xkb_info->config.mk_curve = xkb_info->xkb->ctrls->mk_curve = 0;
    }
    else {
	xkb_info->config.initial_ctrls &= ~(XkbMouseKeysMask |
					    XkbMouseKeysAccelMask);
	xkb_info->xkb->ctrls->enabled_ctrls &= ~(XkbMouseKeysMask |
						 XkbMouseKeysAccelMask);
    }

    /* Enable RepeatKeys */
    XtSetArg(args[0], XtNstate, &state);
    XtGetValues(repeat, args, 1);
    if (state) {
	xkb_info->config.initial_ctrls |= XkbRepeatKeysMask;
	xkb_info->xkb->ctrls->enabled_ctrls |= XkbRepeatKeysMask;
	xkb_info->config.repeat_interval =
	xkb_info->xkb->ctrls->repeat_interval = repeatRate->value * 1000;
	xkb_info->config.repeat_delay =
	xkb_info->xkb->ctrls->repeat_delay = repeatDelay->value * 1000;
    }
    else {
	xkb_info->config.initial_ctrls &= ~XkbRepeatKeysMask;
	xkb_info->xkb->ctrls->enabled_ctrls &= ~XkbRepeatKeysMask;
    }

    /* Enable SlowKeys */
    XtSetArg(args[0], XtNstate, &state);
    XtGetValues(slowToggle, args, 1);
    if (state) {
	xkb_info->config.initial_ctrls |= XkbSlowKeysMask;
	xkb_info->xkb->ctrls->enabled_ctrls |= XkbSlowKeysMask;
	xkb_info->config.slow_keys_delay =
	xkb_info->xkb->ctrls->slow_keys_delay = slow->value * 1000;
    }
    else {
	xkb_info->config.initial_ctrls &= ~XkbSlowKeysMask;
	xkb_info->xkb->ctrls->enabled_ctrls &= ~XkbSlowKeysMask;
    }
    XtSetArg(args[0], XtNstate, &state);
    XtGetValues(slowPressed, args, 1);
    if (state) {
	xkb_info->config.initial_opts |= XkbAX_SKPressFBMask;
	xkb_info->xkb->ctrls->ax_options |= XkbAX_SKPressFBMask;
    }
    else {
	xkb_info->config.initial_opts &= ~XkbAX_SKPressFBMask;
	xkb_info->xkb->ctrls->ax_options &= ~XkbAX_SKPressFBMask;
    }
    XtSetArg(args[0], XtNstate, &state);
    XtGetValues(slowAccepted, args, 1);
    if (state) {
	xkb_info->config.initial_opts |= XkbAX_SKAcceptFBMask;
	xkb_info->xkb->ctrls->ax_options |= XkbAX_SKAcceptFBMask;
    }
    else {
	xkb_info->config.initial_opts &= ~XkbAX_SKAcceptFBMask;
	xkb_info->xkb->ctrls->ax_options &= ~XkbAX_SKAcceptFBMask;
    }

    /* Enable BounceKeys */
    XtSetArg(args[0], XtNstate, &state);
    XtGetValues(bounceToggle, args, 1);
    if (state) {
	xkb_info->config.initial_ctrls |= XkbBounceKeysMask;
	xkb_info->xkb->ctrls->enabled_ctrls |= XkbBounceKeysMask;
	xkb_info->config.debounce_delay =
	xkb_info->xkb->ctrls->debounce_delay = bounce->value * 1000;
    }
    else {
	xkb_info->config.initial_ctrls &= ~XkbBounceKeysMask;
	xkb_info->xkb->ctrls->enabled_ctrls &= ~XkbBounceKeysMask;
    }

    XkbSetControls(DPY, XkbAllControlsMask, xkb_info->xkb);
    XSync(DPY, False);
    (void)UpdateKeyboard(True);
}
