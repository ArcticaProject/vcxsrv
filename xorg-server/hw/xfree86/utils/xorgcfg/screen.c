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
#include <X11/extensions/shape.h>
#include <X11/Xaw/Simple.h>
#include "screen.h"

#define CW	1
#define	CCW	-1

/*
 * Prototypes
 */
void ReshapeScreenWidget(xf86cfgScreen*);
static int qcmp_screen(_Xconst void*, _Xconst void*);

/*
 * Initialization
 */
extern Widget work;

static int rows, columns;	/* number of rows/columns of monitors */

static int mon_width, mon_height;
static int *mon_widths, *mon_heights;

/*
 * Implementation
 */
void
SetScreenRotate(xf86cfgScreen *screen)
{
    static char *Rotate = "Rotate", *_CW = "CW", *_CCW = "CCW";
    int rotate = 0;
    XF86OptionPtr option, options;

    /* This is the only place where xf86cfg is intrusive, and deletes options
     * added by the user directly in the config file. The "Rotate" option
     * will be kept in the screen section.
     */
    if (screen->monitor != NULL) {
	options = ((XF86ConfMonitorPtr)(screen->monitor->config))->mon_option_lst;
	if ((option = xf86findOption(options, Rotate)) != NULL) {
	    if (option->opt_val != NULL)
		rotate = strcasecmp(option->opt_val, _CW) == 0 ? CW :
			 strcasecmp(option->opt_val, _CCW) == 0 ? CCW : 0;
	    xf86removeOption(&((XF86ConfMonitorPtr)(screen->monitor->config))
			     ->mon_option_lst, Rotate);
	}
    }
    if (screen->card != NULL) {
	options = ((XF86ConfDevicePtr)(screen->card->config))->dev_option_lst;
	if ((option = xf86findOption(options, Rotate)) != NULL) {
	    if (option->opt_val != NULL)
		rotate += strcasecmp(option->opt_val, _CW) == 0 ? CW :
			  strcasecmp(option->opt_val, _CCW) == 0 ? CCW : 0;
	    xf86removeOption(&((XF86ConfDevicePtr)(screen->card->config))
			     ->dev_option_lst, Rotate);
	}
    }

    options = screen->screen->scrn_option_lst;
    if ((option = xf86findOption(options, Rotate)) != NULL) {
	if (option->opt_val != NULL)
	    rotate += strcasecmp(option->opt_val, _CW) == 0 ? CW :
		      strcasecmp(option->opt_val, _CCW) == 0 ? CCW : 0;
	xf86removeOption(&screen->screen->scrn_option_lst, Rotate);
    }

    rotate = rotate > 0 ? CW : rotate < 0 ? CCW : 0;
    if (rotate)
	screen->screen->scrn_option_lst =
	    xf86addNewOption(screen->screen->scrn_option_lst,
			     XtNewString(Rotate),
			     XtNewString(rotate > 0 ? _CW : _CCW));
    screen->rotate = rotate;
}

void
CreateScreenWidget(xf86cfgScreen *screen)
{
    Widget w = XtCreateWidget("screen", simpleWidgetClass,
			      XtParent(computer.cpu), NULL, 0);

    SetScreenRotate(screen);
    XtRealizeWidget(w);
    screen->widget = w;
    screen->column = screen->row = -1;

    ReshapeScreenWidget(screen);
}

void
ReshapeScreenWidget(xf86cfgScreen *screen)
{
    Pixmap pixmap;
    XGCValues values;
    GC gc;
    int x = 0, y = 0, width = screen->rect.width, height = screen->rect.height;
    Widget w = screen->widget;

    if (screen->state == USED && screen->row >= 0) {
	if (screen->column == 0)
	    x = w->core.width - width;
	else if (screen->column == columns - 1)
	    x = w->core.width - mon_widths[screen->column];
	else
	    x = (w->core.width - mon_widths[screen->column]) +
		((mon_widths[screen->column] - width) >> 1);

	if (screen->row == 0)
	    y = w->core.height - height;
	else if (screen->row == rows - 1)
	    y = w->core.height - mon_heights[screen->row];
	else
	    y = (w->core.height - mon_heights[screen->row]) +
		((mon_heights[screen->row] - height) >> 1);
    }
    else if (screen->rect.width == 0) {
	width = w->core.width;
	height = w->core.height;
    }

    screen->rect.x = x;
    screen->rect.y = y;
    screen->rect.width = width;
    screen->rect.height = height;
    pixmap = XCreatePixmap(XtDisplay(w), XtWindow(w),
			   w->core.width, w->core.height, 1);
    values.foreground = 0;
    values.background = 1;
    gc = XCreateGC(XtDisplay(w), pixmap, GCForeground | GCBackground, &values);
    XFillRectangle(XtDisplay(w), pixmap, gc, 0, 0, w->core.width, w->core.height);
    XSetForeground(XtDisplay(w), gc, 1);

    DrawScreenMask(XtDisplay(w), pixmap, gc, x, y, x + width, y + height,
		   screen->rotate);
    XShapeCombineMask(XtDisplay(w), XtWindow(w), ShapeBounding, 
		      0, 0, pixmap, ShapeSet);

    /* Do not call XtSetValues, to avoid all extra code for caching pixmaps */
    XFreePixmap(XtDisplay(w), pixmap);
    if (XtIsRealized(w)) {
	pixmap = XCreatePixmap(XtDisplay(w), XtWindow(w),
			       w->core.width, w->core.height,
			       DefaultDepthOfScreen(XtScreen(w)));
	DrawScreen(XtDisplay(w), pixmap, x, y, x + width, y + height,
		   screen->state == USED ? True : False, screen->rotate);
	XSetWindowBackgroundPixmap(XtDisplay(w), XtWindow(w), pixmap);
	XClearWindow(XtDisplay(w), XtWindow(w));
	XFreePixmap(XtDisplay(w), pixmap);
    }
    XFreeGC(XtDisplay(w), gc);
}

void
AddScreen(xf86cfgDevice *mon, xf86cfgDevice *dev)
{
    int nscreens = 0;
    char screen_name[48];
    XF86ConfScreenPtr screen = XF86Config->conf_screen_lst;
    XF86ConfAdjacencyPtr adj;

    while (screen != NULL) {
	++nscreens;
	screen = (XF86ConfScreenPtr)(screen->list.next);
    }
    do {
	XmuSnprintf(screen_name, sizeof(screen_name), "Screen%d",
		    nscreens);
	++nscreens;
    } while (xf86findScreen(screen_name,
	     XF86Config->conf_screen_lst) != NULL);

    screen = (XF86ConfScreenPtr)XtCalloc(1, sizeof(XF86ConfScreenRec));
    screen->scrn_identifier = XtNewString(screen_name);
    screen->scrn_device_str = XtNewString(((XF86ConfDevicePtr)(dev->config))->dev_identifier);
    screen->scrn_device = (XF86ConfDevicePtr)(dev->config);
    screen->scrn_monitor_str = XtNewString(((XF86ConfMonitorPtr)(mon->config))->mon_identifier);
    screen->scrn_monitor = (XF86ConfMonitorPtr)(mon->config);
    XF86Config->conf_screen_lst =
	xf86addScreen(XF86Config->conf_screen_lst, screen);

    adj = (XF86ConfAdjacencyPtr)XtCalloc(1, sizeof(XF86ConfAdjacencyRec));
    adj->adj_screen = screen;
    adj->adj_screen_str = XtNewString(screen_name);
    if (computer.layout == NULL)
	computer.layout = XF86Config->conf_layout_lst = (XF86ConfLayoutPtr)
	    XtCalloc(1, sizeof(XF86ConfLayoutRec));
    computer.layout->lay_adjacency_lst = (XF86ConfAdjacencyPtr)
	xf86addListItem((GenericListPtr)computer.layout->lay_adjacency_lst,
		    (GenericListPtr)adj);

    computer.screens = (xf86cfgScreen**)
	XtRealloc((XtPointer)computer.screens, sizeof(xf86cfgScreen*) *
		  (computer.num_screens + 1));
    computer.screens[computer.num_screens] =
	(xf86cfgScreen*)XtCalloc(1, sizeof(xf86cfgScreen));
    computer.screens[computer.num_screens]->screen = screen;
    computer.screens[computer.num_screens]->card = dev;
    computer.screens[computer.num_screens]->monitor = mon;

    ++dev->refcount;
    ++mon->refcount;

    CreateScreenWidget(computer.screens[computer.num_screens]);
    computer.screens[computer.num_screens]->type = SCREEN;
    SetTip((xf86cfgDevice*)computer.screens[computer.num_screens]);

    ++computer.num_screens;
}

void
RemoveScreen(xf86cfgDevice *mon, xf86cfgDevice *dev)
{
    XF86ConfScreenPtr screen = XF86Config->conf_screen_lst;
    int i;

    mon->state = dev->state = UNUSED;
    while (screen != NULL) {
	if ((XtPointer)screen->scrn_monitor == mon->config &&
	    (XtPointer)screen->scrn_device == dev->config)
	    break;

	screen = (XF86ConfScreenPtr)(screen->list.next);
    }
    --mon->refcount;
    --dev->refcount;

    for (i = 0; i < computer.num_screens; i++) {
	if (computer.screens[i]->screen == screen) {
	    XtDestroyWidget(computer.screens[i]->widget);
	    if (i < --computer.num_screens)
		memmove(&computer.screens[i], &computer.screens[i + 1],
			(computer.num_screens - i) * sizeof(xf86cfgScreen*));
	    break;
	}
    }

    xf86removeScreen(XF86Config, screen);
}

void
ChangeScreen(XF86ConfMonitorPtr mon, XF86ConfMonitorPtr oldmon,
	     XF86ConfDevicePtr dev, XF86ConfDevicePtr olddev)
{
    int ioldm, im, ioldc, ic;

    if (mon == oldmon && dev == olddev)
	return;

    if (mon != NULL) {
	for (im = 0; im < computer.num_devices; im++)
	    if (computer.devices[im]->config == (XtPointer)mon)
		break;
    }
    else
	im = -1;
    if (oldmon != NULL) {
	for (ioldm = 0; ioldm < computer.num_devices; ioldm++)
	    if (computer.devices[ioldm]->config == (XtPointer)oldmon)
		break;
    }
    else
	ioldm = -1;

    if (dev != NULL) {
	for (ic = 0; ic < computer.num_devices; ic++)
	    if (computer.devices[ic]->config == (XtPointer)dev)
		break;
    }
    else
	ic = -1;
    if (olddev != NULL) {
	for (ioldc = 0; ioldc < computer.num_devices; ioldc++)
	    if (computer.devices[ioldc]->config == (XtPointer)olddev)
		break;
    }
    else
	ioldc = -1;

    if (ioldm >= 0 && ioldc >= 0) {
	RemoveScreen(computer.devices[ioldm], computer.devices[ioldc]);
	computer.devices[ioldm]->state = UNUSED;
/*	computer.devices[ioldc]->state = UNUSED;*/
    }

    if (im >= 0 && ic >= 0) {
	AddScreen(computer.devices[im], computer.devices[ic]);
	computer.devices[im]->state = USED;
/*	computer.devices[ic]->state = USED;*/
    }
}

/*

+------------------------------------------------+
|						 |
|  +------------------------------------------+  |
|  |					      |	 |
|  |					      |	 |
|  |					      |	 |
|  |					      |	 |
|  |					      |	 |
|  |					      |	 |
|  |					      |	 |
|  |					      |	 |
|  |					      |	 |
|  |					      |	 |
|  |					      |	 |
|  |					      |	 |
|  |					      |	 |
|  |					      |	 |
|  +------------------------------------------+  |
|						 |
+------------------------------------------------+
	    |			     |
    +-------+			     +-------+
    |					     |
    +----------------------------------------+

 */
static double oxs = 0.0, oys = 0.0, oxe = 100.0, oye = 70.0;
static double ixs = 7.0, iys = 7.0, ixe = 93.0, iye = 63.0;
static double lin[] = { 25.0, 70.0, 25.0, 75.0,  5.0, 75.0,  5.0, 80.0,
			95.0, 80.0, 95.0, 75.0, 75.0, 75.0, 75.0, 70.0 };

void
DrawScreen(Display *dpy, Drawable win, int xs, int ys, int xe, int ye,
	   Bool active, int rotate)
{
    double xfact, yfact;
    XPoint points[(sizeof(lin) / sizeof(lin[0])) >> 1];
    int i;
    static GC gray0, gray1, gray2, black, red;

    if (black == NULL) {
	XColor color, exact;
	XGCValues values;

	XAllocNamedColor(XtDisplay(toplevel), toplevel->core.colormap, "gray95",
			 &color, &exact);
	values.foreground = color.pixel;
	gray0 = XCreateGC(XtDisplay(toplevel), win, GCForeground, &values);
	XAllocNamedColor(XtDisplay(toplevel), toplevel->core.colormap, "gray75",
			 &color, &exact);
	values.foreground = color.pixel;
	gray1 = XCreateGC(XtDisplay(toplevel), win, GCForeground, &values);

	XAllocNamedColor(XtDisplay(toplevel), toplevel->core.colormap, "gray60",
			 &color, &exact);
	values.foreground = color.pixel;
	gray2 = XCreateGC(XtDisplay(toplevel), win, GCForeground, &values);

	XAllocNamedColor(XtDisplay(toplevel), toplevel->core.colormap, "gray20",
			 &color, &exact);
	values.foreground = color.pixel;
	black = XCreateGC(XtDisplay(toplevel), win, GCForeground, &values);

	XAllocNamedColor(XtDisplay(toplevel), toplevel->core.colormap, "red",
			 &color, &exact);
	values.foreground = color.pixel;
	values.line_width = 4;
	values.cap_style = CapButt;
	red = XCreateGC(XtDisplay(toplevel), win,
			GCForeground | GCLineWidth | GCCapStyle, &values);
    }

    if (rotate) {
	xfact = (xe - xs) / 80.0;
	yfact = (ye - ys) / 100.0;
	if (rotate == CW) {
	    /* outer rectangle */
	    XFillRectangle(dpy, win, gray1,
			   oxs * xfact + xs + .5,
			   oys * yfact + ys + .5,
			   (oye - oys) * xfact + .5,
			   (oxe - oxs) * yfact + .5);
	    XDrawLine(dpy, win, gray2,
		      xs, ye - 1,
		      70 * xfact + xs - 1 + .5, ye - 1);
	    XDrawLine(dpy, win, gray2,
		      70 * xfact + xs - 1 + .5, ye - 1,
		      70 * xfact + xs - 1 + .5, ys);
	    /* inner rectangle */
	    XFillRectangle(dpy, win, black,
			   ixs * xfact + xs + .5,
			   iys * yfact + ys + .5,
			   (iye - iys) * xfact + .5,
			   (ixe - ixs) * yfact + .5);
	    for (i = 0; i < sizeof(points) / sizeof(points[0]); i++) {
		points[i].x = lin[(i<<1) + 1] * xfact + xs + .5;
		points[i].y = lin[(i<<1)] * yfact + ys + .5;
	    }
	    XFillPolygon(dpy, win, gray2, points, i, Convex, CoordModeOrigin);
	    XDrawLine(dpy, win, gray0,
		      (oxe - 10) * xfact + xs + .5, oys * yfact + ys + .5,
		      xs, oys * yfact + ys + .5);
	    XDrawLine(dpy, win, gray0,
		      xs, ys,
		      xs, xe);
	    XDrawLine(dpy, win, black,
		      lin[7] * xfact + xs - 1 + .5, lin[6] * yfact + ys + .5,
		      lin[9] * xfact + xs - 1 + .5, lin[8] * yfact + ys - 1 + .5);
	    XDrawLine(dpy, win, black,
		      lin[9] * xfact + xs - 1 + .5, lin[8] * yfact + ys - 1 + .5,
		      lin[11] * xfact + xs + .5, lin[10] * yfact + ys - 1 + .5);
	    XDrawLine(dpy, win, black,
		      lin[13] * xfact + xs + .5, lin[12] * yfact + ys - 1 + .5,
		      lin[15] * xfact + xs + .5, lin[14] * yfact + ys - 1 + .5);

	    if (!active) {
		XDrawLine(dpy, win, red,
			  iys * xfact, ixs * yfact, iye * xfact, ixe * yfact);
		XDrawLine(dpy, win, red,
			  iye * xfact, ixs * yfact, iys * xfact, ixe * yfact);
	    }
	}
	else if (rotate == CCW) {
	    /* outer rectangle */
	    XFillRectangle(dpy, win, gray1,
			   10 * xfact + xs + .5,
			   oys * yfact + ys + .5,
			   (oye - oys) * xfact + .5,
			   (oxe - oxs) * yfact + .5);

	    XDrawLine(dpy, win, gray2,
		      10 * xfact + xs + .5, ye - 1,
		      oxe * xfact + xs - 1 + .5, ye - 1);
	    XDrawLine(dpy, win, gray2,
		      xe - 1, ye - 1,
		      xe - 1, ys);
	    /* inner rectangle */
	    XFillRectangle(dpy, win, black,
			   (ixs + 10) * xfact + xs + .5,
			   iys * yfact + ys + .5,
			   (iye - iys) * xfact + .5,
			   (ixe - ixs) * yfact + .5);
	    for (i = 0; i < sizeof(points) / sizeof(points[0]); i++) {
		points[i].x = (-lin[(i<<1) + 1] + 80.0) * xfact + xs + .5;
		points[i].y = lin[(i<<1)] * yfact + ys + .5;
	    }
	    XFillPolygon(dpy, win, gray2, points, i, Convex, CoordModeOrigin);
	    XDrawLine(dpy, win, gray0,
		      oxe * xfact + xs + .5, oys * yfact + ys + .5,
		      (oxs - 10) * xfact + xs + .5, oys * yfact + ys + .5);
	    XDrawLine(dpy, win, gray0,
		      (oxs + 10) * xfact + xs + .5, ys,
		      (oxs + 10) * xfact + xs + .5, xe);

	    XDrawLine(dpy, win, black,
		      xs, lin[8] * yfact - 1 + ys + .5,
		      4 * xfact + xs + .5, lin[8] * yfact - 1 + ys + .5);
	    XDrawLine(dpy, win, black,
		      4 * xfact + xs, lin[8] * yfact - 1 + ys + .5,
		      4 * xfact + xs, lin[3] * yfact - 1 + ys + .5);
	    XDrawLine(dpy, win, black,
		      4 * xfact + xs + .5, lin[3] * yfact - 1 + ys + .5,
		      10 * xfact + xs + .5 - 1, lin[3] * yfact - 1 + ys + .5);
	    XDrawLine(dpy, win, black,
		      4 * xfact + xs, lin[0] * yfact - 1 + ys + .5,
		      4 * xfact + xs, lin[4] * yfact - 1 + ys + .5);

	    if (!active) {
		XDrawLine(dpy, win, red,
			  (iys + 10) * xfact, ixs * yfact,
			  (iye + 10) * xfact, ixe * yfact);
		XDrawLine(dpy, win, red,
			  (iye + 10) * xfact, ixs * yfact,
			  (iys + 10) * xfact, ixe * yfact);
	    }
	}
    }
    else {
	xfact = (xe - xs) / 100.0;
	yfact = (ye - ys) / 80.0;

	/* outer rectangle */
	XFillRectangle(dpy, win, gray1,
		       oxs * xfact + xs + .5,
		       oys * yfact + ys + .5,
		       (oxe - oxs) * xfact + .5,
		       (oye - oys) * yfact + .5);

	XDrawLine(dpy, win, gray2,
		  oxs * xfact + xs + .5, oye * yfact + ys - 1 + .5,
		  oxe * xfact + xs - 1 + .5, oye * yfact + ys - 1 + .5);
	XDrawLine(dpy, win, gray2,
		  oxe * xfact + xs - 1 + .5, oys * yfact + ys + .5,
		  oxe * xfact + xs - 1 + .5, oye * yfact + ys - 1 + .5);

	/* inner rectangle */
	XFillRectangle(dpy, win, black,
		       ixs * xfact + xs + .5,
		       iys * yfact + ys + .5,
		       (ixe - ixs) * xfact + .5,
		       (iye - iys) * yfact + .5);

	for (i = 0; i < sizeof(points) / sizeof(points[0]); i++) {
	    points[i].x = lin[i<<1] * xfact + xs + .5;
	    points[i].y = lin[(i<<1) + 1] * yfact + ys + .5;
	}

	XFillPolygon(dpy, win, gray2, points, i, Convex, CoordModeOrigin);

	XDrawLine(dpy, win, black,
		  lin[6] * xfact + xs + .5, lin[7] * yfact + ys - 1 + .5,
		  lin[8] * xfact + xs - 1 + .5, lin[9] * yfact + ys - 1 + .5);
	XDrawLine(dpy, win, black,
		  lin[8] * xfact + xs - 1 + .5, lin[9] * yfact + ys - 1 + .5,
		  lin[10] * xfact + xs - 1 + .5, lin[11] * yfact + ys + .5);
	XDrawLine(dpy, win, black,
		  lin[12] * xfact + xs - 1 + .5, lin[13] * yfact + ys + .5,
		  lin[14] * xfact + xs - 1 + .5, lin[15] * yfact + ys + .5);

	XDrawLine(dpy, win, gray0,
		  oxe * xfact + xs + .5, oys * yfact + ys + .5,
		  oxs * xfact + xs + .5, oys * yfact + ys + .5);
	XDrawLine(dpy, win, gray0,
		  oxs * xfact + xs + .5, oys * yfact + ys + .5,
		  oxs * xfact + xs + .5, lin[1] * yfact + ys + .5);

	if (!active) {
	    XDrawLine(dpy, win, red,
		      ixs * xfact, iys * yfact, ixe * xfact, iye * yfact);
	    XDrawLine(dpy, win, red,
		      ixe * xfact, iys * yfact, ixs * xfact, iye * yfact);
	}
    }
}

void
DrawScreenMask(Display *dpy, Drawable win, GC gc, int xs, int ys, int xe, int ye,
	       int rotate)
{
    double xfact, yfact;
    XPoint points[(sizeof(lin) / sizeof(lin[0])) >> 1];
    int i = 0, x = 0, y = 0, width, height;

    if (rotate) {
	xfact = (xe - xs) / 80.0;
	yfact = (ye - ys) / 100.0;
	width = (oye - oys) * xfact + .5;
	height = (oxe - oxs) * yfact + .5;
	if (rotate == CW) {
	    x = oxs * xfact + xs + .5;
	    y = oys * yfact + ys + .5;
	    for (i = 0; i < sizeof(points) / sizeof(points[0]); i++) {
		points[i].x = lin[(i<<1) + 1] * xfact + xs + .5;
		points[i].y = lin[(i<<1)] * yfact + ys + .5;
	    }
	}
	else if (rotate == CCW) {
	    x = 10 * xfact + xs + .5;
	    y = oys * yfact + ys + .5;
	    for (i = 0; i < sizeof(points) / sizeof(points[0]); i++) {
		points[i].x = (-lin[(i<<1) + 1] + 80.0) * xfact + xs + .5;
		points[i].y = lin[(i<<1)] * yfact + ys + .5;
	    }
	}
    }
    else {
	xfact = (xe - xs) / 100.0;
	yfact = (ye - ys) / 80.0;
	x = oxs * xfact + xs + .5;
	y = oys * yfact + ys + .5;
	width = (oxe - oxs) * xfact + .5;
	height = (oye - oys) * yfact + .5;
	for (i = 0; i < sizeof(points) / sizeof(points[0]); i++) {
	    points[i].x = lin[(i<<1)] * xfact + xs + .5;
	    points[i].y = lin[(i<<1) + 1] * yfact + ys + .5;
	}
    }

    /* rectangle */
    XFillRectangle(dpy, win, gc, x, y, width, height);


    XFillPolygon(dpy, win, gc, points, i, Convex, CoordModeOrigin);
}

void
AdjustScreenUI(void)
{
    XF86ConfLayoutPtr lay = computer.layout;
    XF86ConfAdjacencyPtr adj;
    int i, dx, dy, x, y, w, h, base = -1;
    double xf, yf;

    if (lay == NULL)
	return;

    adj = lay->lay_adjacency_lst;

#define USED1	-USED

    XtFree((XtPointer)mon_widths);
    XtFree((XtPointer)mon_heights);
    mon_widths = (int*)XtCalloc(1, sizeof(int) * columns);
    mon_heights = (int*)XtCalloc(1, sizeof(int) * rows);

    mon_width = mon_height = 0;
    for (i = 0; i < computer.num_screens; i++) {
	if (base == -1 && computer.screens[i]->state == USED)
	    base = i;
	if (computer.screens[i]->screen->scrn_monitor->mon_width > mon_width)
	    mon_width = computer.screens[i]->screen->scrn_monitor->mon_width;
	if (computer.screens[i]->screen->scrn_monitor->mon_height > mon_height)
	    mon_height = computer.screens[i]->screen->scrn_monitor->mon_height;
    }
    if (base < 0) {
	for (i = 0; i < computer.num_screens; i++)
	    ReshapeScreenWidget(computer.screens[i]);
	return;
    }

    if (mon_width == 0) {
	mon_width = 10;
	mon_height = 8;
    }

    XtUnmapWidget(work);

    while (adj) {
	xf86cfgScreen *scr = NULL,
	    *topscr = NULL, *botscr = NULL, *lefscr = NULL, *rigscr = NULL;

	for (i = 0; i < computer.num_screens; i++)
	    if (computer.screens[i]->screen == adj->adj_screen)
		break;
	if (i < computer.num_screens)
	    scr = computer.screens[i];

	if (adj->adj_top != NULL) {
	    for (i = 0; i < computer.num_screens; i++)
		if (computer.screens[i]->screen == adj->adj_top)
		    break;
	    if (i < computer.num_screens)
		topscr = computer.screens[i];
	}

	if (adj->adj_bottom != NULL) {
	    for (i = 0; i < computer.num_screens; i++)
		if (computer.screens[i]->screen == adj->adj_bottom)
		    break;
	    if (i < computer.num_screens)
		botscr = computer.screens[i];
	}

	if (adj->adj_left != NULL) {
	    for (i = 0; i < computer.num_screens; i++)
		if (computer.screens[i]->screen == adj->adj_left)
		    break;
	    if (i < computer.num_screens)
		lefscr = computer.screens[i];
	}

	if (adj->adj_right != NULL) {
	    for (i = 0; i < computer.num_screens; i++)
		if (computer.screens[i]->screen == adj->adj_right)
		    break;
	    if (i < computer.num_screens)
		rigscr = computer.screens[i];
	}

	if (lefscr == NULL && rigscr == NULL && topscr == NULL && lefscr == NULL) {
	    XF86ConfScreenPtr s;

	    if (adj->adj_where >= CONF_ADJ_RIGHTOF && adj->adj_where <= CONF_ADJ_BELOW) {
		s = xf86findScreen(adj->adj_refscreen, XF86Config->conf_screen_lst);
		for (i = 0; i < computer.num_screens; i++)
		    if (computer.screens[i]->screen == s)
			break;
		if (i < computer.num_screens) {
		    switch (adj->adj_where) {
			case CONF_ADJ_RIGHTOF:
			    lefscr = computer.screens[i];
			    break;
			case CONF_ADJ_LEFTOF:
			    rigscr = computer.screens[i];
			    break;
			case CONF_ADJ_ABOVE:
			    botscr = computer.screens[i];
			    break;
			case CONF_ADJ_BELOW:
			    topscr = computer.screens[i];
			    break;
		    }
		}
	    }
	}

	XtMoveWidget(scr->widget, 0, 0);
	scr->state = USED1;
	if (lefscr != NULL) {
	    if (lefscr->state == USED1)
		XtMoveWidget(scr->widget,
			     lefscr->widget->core.x + lefscr->widget->core.width,
			     lefscr->widget->core.y);
	    else
		XtMoveWidget(lefscr->widget,
			     -(int)(lefscr->widget->core.width),
			     scr->widget->core.y);
	}

	if (rigscr != NULL) {
	    if (rigscr->state == USED1) {
		dx = rigscr->widget->core.x - scr->widget->core.width - scr->widget->core.x;
		dy = rigscr->widget->core.y - scr->widget->core.y;

		XtMoveWidget(scr->widget, scr->widget->core.x + dx,
			     scr->widget->core.y + dy);
		if (lefscr != NULL && lefscr->state != USED1)
		    XtMoveWidget(lefscr->widget, lefscr->widget->core.x + dx,
				 lefscr->widget->core.y + dy);
	    }
	    else
		XtMoveWidget(rigscr->widget, scr->widget->core.width,
			     scr->widget->core.y);
	}

	if (topscr != NULL) {
	    if (topscr->state == USED1) {
		dx = topscr->widget->core.x - scr->widget->core.x;
		dy = topscr->widget->core.y + topscr->widget->core.height -
		     scr->widget->core.y;

		XtMoveWidget(scr->widget, scr->widget->core.x + dx,
			     scr->widget->core.y + dy);
		if (lefscr != NULL && lefscr->state != USED1)
		    XtMoveWidget(lefscr->widget, lefscr->widget->core.x + dx,
				 lefscr->widget->core.y + dy);
		if (rigscr != NULL && rigscr->state != USED1)
		    XtMoveWidget(rigscr->widget, rigscr->widget->core.x + dx,
				 rigscr->widget->core.y + dy);
	    }
	    else
		XtMoveWidget(topscr->widget, scr->widget->core.x,
			     scr->widget->core.y - topscr->widget->core.height);
	}

	if (botscr != NULL) {
	    if (botscr->state == USED1) {
		dx = botscr->widget->core.x - scr->widget->core.x;
		dy = botscr->widget->core.y - scr->widget->core.height - scr->widget->core.y;

		XtMoveWidget(scr->widget, scr->widget->core.x + dx,
			     scr->widget->core.y + dy);
		if (lefscr != NULL && lefscr->state != USED1)
		    XtMoveWidget(lefscr->widget, lefscr->widget->core.x + dx,
				 lefscr->widget->core.y + dy);
		if (rigscr != NULL && rigscr->state != USED1)
		    XtMoveWidget(rigscr->widget, rigscr->widget->core.x + dx,
				 rigscr->widget->core.y + dy);
		if (botscr != NULL && botscr->state != USED1)
		    XtMoveWidget(botscr->widget, botscr->widget->core.x + dx,
				 botscr->widget->core.y + dy);
	    }
	    else
		XtMoveWidget(botscr->widget, scr->widget->core.x,
			     scr->widget->core.y + scr->widget->core.height);
	}

	adj = (XF86ConfAdjacencyPtr)(adj->list.next);
    }

    for (i = 0; i < computer.num_screens; i++)
	if (computer.screens[i]->state == USED1)
	    computer.screens[i]->state = USED;
	else
	    XLowerWindow(XtDisplay(computer.screens[i]->widget),
			 XtWindow(computer.screens[i]->widget));

    w = work->core.width / (columns + 1) - 5;
    h = work->core.height / (rows + 1) - 5;

    if (w > h)
	w = h;
    else
	h = w;

    dx = (work->core.width - (columns * w)) >> 1;
    dy = (work->core.height - (rows * h)) >> 1;

    xf = (double)w / (double)computer.screens[0]->widget->core.width;
    yf = (double)h / (double)computer.screens[0]->widget->core.height;

    for (i = 0; i < computer.num_screens; i++) {
	Widget z = computer.screens[i]->widget;

	if (computer.screens[i]->state == USED)
	    XtConfigureWidget(z, z->core.x * xf + dx,
			      z->core.y * yf + dy, w, h, 0);
	else
	    XtConfigureWidget(z, z->core.x, z->core.y, w, h, 0);
    }

    if (computer.screens[base]->row >= 0) {
	double xf, yf;
	int width, height;

	for (i = 0; i < computer.num_screens; i++) {
	    width = computer.screens[i]->screen->scrn_monitor->mon_width;
	    height = computer.screens[i]->screen->scrn_monitor->mon_height;
	    if (width <= 0) {
		width = mon_width;
		height = mon_height;
	    }

	    if (computer.screens[i]->rotate) {
		xf = (double)width / (double)mon_width * 8. / 10.;
		yf = (double)height / (double)mon_height;
	    }
	    else {
		xf = (double)width / (double)mon_width;
		yf = (double)height / (double)mon_height * 8. / 10.;
	    }
	    width = computer.screens[i]->widget->core.width * xf;
	    height = computer.screens[i]->widget->core.height * yf;
	    if (computer.screens[i]->state == USED) {
		if (mon_widths[computer.screens[i]->column] < width)
		    mon_widths[computer.screens[i]->column] = width;
		if (mon_heights[computer.screens[i]->row] < height)
		    mon_heights[computer.screens[i]->row] = height;
	    }

	    /* do it here to avoid recalculation */
	    computer.screens[i]->rect.width = width;
	    computer.screens[i]->rect.height = height;
	}
    }

    for (i = 0; i < computer.num_screens; i++)
	ReshapeScreenWidget(computer.screens[i]);

    /* do a new pass, to avoid gaps if the monitors have different
     * sizes.
     */
    if (computer.screens[base]->row >= 0) {
	x = computer.screens[base]->widget->core.x;
	y = computer.screens[base]->widget->core.y;

	/* screens representations are already ordered */
	for (i = base; i < computer.num_screens; i++) {
	    if (computer.screens[i]->state == UNUSED)
		continue;
	    if (computer.screens[i]->column != 0)
		x += mon_widths[computer.screens[i]->column];
	    else {
		x = computer.screens[base]->widget->core.x;
		if (i != base)
		    y += mon_heights[computer.screens[i]->row];
	    }
	    XtMoveWidget(computer.screens[i]->widget, x, y);
	}
    }
    XtMapWidget(work);
}

static int
qcmp_screen(_Xconst void *a, _Xconst void *b)
{
    xf86cfgScreen *s1, *s2;

    s1 = *(xf86cfgScreen**)a;
    s2 = *(xf86cfgScreen**)b;

    if (s1->widget->core.x > s2->widget->core.x) {
	if (s2->widget->core.y >=
	    s1->widget->core.y + (s1->widget->core.height >> 1))
	    return (-1);
	return (1);
    }
    else {
	if (s1->widget->core.y >=
	    s2->widget->core.y + (s2->widget->core.height >> 1))
	    return (1);
	return (-1);
    }
    /*NOTREACHED*/
}

void
UpdateScreenUI(void)
{
    XF86ConfLayoutPtr lay = computer.layout;
    XF86ConfAdjacencyPtr adj, prev, left, base;
    int i, p, cols, scrno;

    if (lay == NULL)
	return;

    rows = columns = cols = 1;

    qsort(computer.screens, computer.num_screens, sizeof(xf86cfgScreen*),
	  qcmp_screen);

    adj = prev = left = base = NULL;
    for (i = p = scrno = 0; i < computer.num_screens; i++) {
	XF86ConfScreenPtr scr = computer.screens[i]->screen;

	if (computer.screens[i]->state == UNUSED)
	    continue;

	adj = (XF86ConfAdjacencyPtr)XtCalloc(1, sizeof(XF86ConfAdjacencyRec));
	adj->adj_scrnum = scrno++;
	adj->adj_screen = scr;
	adj->adj_screen_str = XtNewString(scr->scrn_identifier);
	if (base == NULL) {
	    base = left = adj;
	    computer.screens[i]->row = computer.screens[i]->column = 0;
	}
	else {
	    int dy = computer.screens[i]->widget->core.y -
		     computer.screens[p]->widget->core.y;

	    prev->list.next = adj;
	    if (dy > (computer.screens[i]->widget->core.height >> 1)) {
		adj->adj_where = CONF_ADJ_BELOW;
		adj->adj_refscreen = XtNewString(left->adj_screen_str);
		left = adj;
		computer.screens[i]->row = rows;
		computer.screens[i]->column = 0;
		cols = 1;
		++rows;
	    }
	    else {
		computer.screens[i]->row = rows - 1;
		computer.screens[i]->column = cols;
		adj->adj_where = CONF_ADJ_RIGHTOF;
		if (++cols > columns)
		    columns = cols;
		adj->adj_refscreen = XtNewString(prev->adj_screen_str);
	    }
	}
	prev = adj;
	p = i;
    }

    adj = lay->lay_adjacency_lst;

    while (adj != NULL) {
	prev = adj;
	adj = (XF86ConfAdjacencyPtr)(adj->list.next);
	XtFree(prev->adj_screen_str);
	XtFree(prev->adj_right_str);
	XtFree(prev->adj_left_str);
	XtFree(prev->adj_top_str);
	XtFree(prev->adj_bottom_str);
	XtFree(prev->adj_refscreen);
	XtFree((char*)prev);
    }

    lay->lay_adjacency_lst = base;
}
