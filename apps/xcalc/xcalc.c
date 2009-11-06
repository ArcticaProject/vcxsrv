/* $XConsortium: xcalc.c,v 1.16 94/04/17 20:43:31 converse Exp $ */
/*

Copyright (c) 1989  X Consortium

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be included
in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE X CONSORTIUM BE LIABLE FOR ANY CLAIM, DAMAGES OR
OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of the X Consortium shall
not be used in advertising or otherwise to promote the sale, use or
other dealings in this Software without prior written authorization
from the X Consortium.

*/
/* $XFree86$ */

/*
 * xcalc.c  -  a hand calculator for the X Window system
 *
 *  Original Author:  John H. Bradley, University of Pennsylvania
 *			(bradley@cis.upenn.edu)  March, 1987
 *  RPN mode added and port to X11 by Mark Rosenstein, MIT Project Athena
 *  Rewritten to be an Xaw and Xt client by Donna Converse, MIT X Consortium
 */

#include <stdio.h>
#include <math.h>
#include <signal.h>
#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <X11/Xatom.h>
#include <X11/Shell.h>
#include <X11/Xaw/Cardinals.h>
#include <X11/Xaw/Form.h>
#include <X11/Xaw/Label.h>
#include <X11/Xaw/Command.h>
#include <X11/Xaw/Toggle.h>
#include <X11/cursorfont.h>
#include "xcalc.h"
#include "actions.h"

#ifndef IEEE
extern signal_t fperr();
extern signal_t illerr();
#endif

/*
 *	global data
 */
int	rpn = 0;		/* Reverse Polish Notation (HP mode) flag */
#define LCD_STR_LEN	32
char	dispstr[LCD_STR_LEN];		/* string to show up in the LCD */
Atom	wm_delete_window;		/* see ICCCM section 5.2.2 */

/*
 *	local data 
 */
static Display	*dpy = NULL;		/* connection to the X server */
static Widget	toplevel=NULL;  	/* top level shell widget */
static Widget   calculator=NULL;	/* an underlying form widget */
static Widget	LCD = NULL;		/* liquid crystal display */
static Widget	ind[6];			/* mode indicators in the screen */
static char	selstr[LCD_STR_LEN]; /* storage for selections from the LCD */
					/* checkerboard used in mono mode */
static XtAppContext xtcontext;		/* Toolkit application context */
#define check_width 16
#define check_height 16
static unsigned char check_bits[] = {
   0x55, 0x55, 0xaa, 0xaa, 0x55, 0x55, 0xaa, 0xaa, 0x55, 0x55, 0xaa, 0xaa,
   0x55, 0x55, 0xaa, 0xaa, 0x55, 0x55, 0xaa, 0xaa, 0x55, 0x55, 0xaa, 0xaa,
   0x55, 0x55, 0xaa, 0xaa, 0x55, 0x55, 0xaa, 0xaa};

/*	command line options specific to the application */
static XrmOptionDescRec Options[] = {
{"-rpn",	"rpn",		XrmoptionNoArg,		(XtPointer)"on"},
{"-stipple",	"stipple",	XrmoptionNoArg,		(XtPointer)"on"}
};

/*	resources specific to the application */
static struct resources {
    Boolean	rpn;		/* reverse polish notation (HP mode) */
    Boolean	stipple;	/* background stipple */
    Cursor	cursor;
} appResources;

#define offset(field) XtOffsetOf(struct resources, field)
static XtResource Resources[] = {
{"rpn",		"Rpn",		XtRBoolean,	sizeof(Boolean),
     offset(rpn),	XtRImmediate,	(XtPointer) False},
{"stipple",	"Stipple",	XtRBoolean,	sizeof(Boolean),
     offset(stipple),	XtRImmediate,	(XtPointer) False},
{"cursor",	"Cursor",	XtRCursor,	sizeof(Cursor),
     offset(cursor),	XtRCursor,	(XtPointer)NULL}
};
#undef offset


int
main(argc, argv)
    int		argc;
    char	**argv;
{
    Arg		args[3];

    void create_calculator();
    void Quit(), Syntax();


    XtSetLanguageProc(NULL, (XtLanguageProc) NULL, NULL);

    toplevel = XtAppInitialize(&xtcontext, "XCalc", Options, XtNumber(Options),
			       &argc, argv, NULL, NULL, 0);
    if (argc != 1) Syntax(argc, argv);
    
    XtSetArg(args[0], XtNinput, True);
    XtSetValues(toplevel, args, ONE);

    XtGetApplicationResources(toplevel, (XtPointer)&appResources, Resources,
			      XtNumber(Resources), (ArgList) NULL, ZERO);

    create_calculator(toplevel);

    XtAppAddActions(xtcontext, Actions, XtNumber(Actions));

    XtOverrideTranslations(toplevel, 
	   XtParseTranslationTable("<Message>WM_PROTOCOLS: quit()\n"));

    XtRealizeWidget(toplevel);

    dpy = XtDisplay(toplevel);
    wm_delete_window = XInternAtom(dpy, "WM_DELETE_WINDOW", False);
    (void) XSetWMProtocols(dpy, XtWindow(toplevel), &wm_delete_window, 1);
    XDefineCursor(dpy, XtWindow(toplevel), appResources.cursor);

    if (appResources.stipple || (CellsOfScreen(XtScreen(toplevel)) <= 2))
    {
	Screen	*screen = XtScreen(toplevel);
	Pixmap	backgroundPix;

	backgroundPix = XCreatePixmapFromBitmapData
	    (dpy, XtWindow(toplevel),
	     (char *)check_bits, check_width, check_height,
	     WhitePixelOfScreen(screen), BlackPixelOfScreen(screen),
	     DefaultDepthOfScreen(screen));
	XtSetArg(args[0], XtNbackgroundPixmap, backgroundPix);
	XtSetValues(calculator, args, ONE);
    }

#ifndef IEEE
    signal(SIGFPE,fperr);
    signal(SIGILL,illerr);
#endif
    ResetCalc();
    XtAppMainLoop(xtcontext);

    return 0;
}

void create_calculator(shell)
    Widget	shell;
{
    void create_display();
    void create_keypad();

    rpn = appResources.rpn;
    calculator = XtCreateManagedWidget(rpn ? "hp" : "ti", formWidgetClass,
				       shell, (ArgList) NULL, ZERO);
    create_display(calculator);
    create_keypad(calculator);
    XtSetKeyboardFocus(calculator, LCD);
}

/*
 *	Do the calculator data display widgets.
 */
void create_display(parent)
    Widget	parent;
{
    Widget	bevel, screen;
    static Arg	args[] = {
    	{XtNborderWidth, (XtArgVal)0},
    	{XtNjustify, (XtArgVal)XtJustifyRight}
    };

    /* the frame surrounding the calculator display */
    bevel = XtCreateManagedWidget("bevel", formWidgetClass, parent,
				  (ArgList) NULL, ZERO);

    /* the screen of the calculator */
    screen = XtCreateManagedWidget("screen", formWidgetClass, bevel,
				   (ArgList) NULL, ZERO);

    /* M - the memory indicator */
    ind[XCalc_MEMORY] = XtCreateManagedWidget("M", labelWidgetClass, screen,
					args, XtNumber(args));

    /* liquid crystal display */
    LCD = XtCreateManagedWidget("LCD", toggleWidgetClass, screen, args,
				XtNumber(args));

    /* INV - the inverse function indicator */
    ind[XCalc_INVERSE] = XtCreateManagedWidget("INV", labelWidgetClass, 
					 screen, args, XtNumber(args));

    /* DEG - the degrees switch indicator */
    ind[XCalc_DEGREE] = XtCreateManagedWidget("DEG", labelWidgetClass, screen,
					args, XtNumber(args));

    /* RAD - the radian switch indicator */
    ind[XCalc_RADIAN] = XtCreateManagedWidget("RAD", labelWidgetClass, screen,
					args, XtNumber(args));

    /* GRAD - the grad switch indicator */
    ind[XCalc_GRADAM] = XtCreateManagedWidget("GRAD", labelWidgetClass, screen,
					args, XtNumber(args));

    /* () - the parenthesis indicator */
    ind[XCalc_PAREN] = XtCreateManagedWidget("P", labelWidgetClass, screen,
					     args, XtNumber(args));
}

/*
 *	Do all the buttons.  The application defaults file will give the
 *	default button placement, default button labels, and default 
 *      actions connected to the buttons.  The user can change any of 
 *      these defaults in an environment-specific resource file.
 */

void create_keypad(parent)
    Widget	parent;
{
    static char	*Keyboard[] = {
	"button1", "button2", "button3", "button4", "button5",
	"button6", "button7", "button8", "button9", "button10",
	"button11","button12","button13","button14","button15",
	"button16","button17","button18","button19","button20",
	"button21","button22","button23","button24","button25",
	"button26","button27","button28","button29","button30",
	"button31","button32","button33","button34","button35",
	"button36","button37","button38","button39","button40"
	};
    register int i;
    int		 n = XtNumber(Keyboard);

    if (appResources.rpn) n--; 	/* HP has 39 buttons, TI has 40 */

    for (i=0; i < n; i++)
	XtCreateManagedWidget(Keyboard[i], commandWidgetClass, parent,
			      (ArgList) NULL, ZERO);
}

/*
 *	Miscellaneous utility routines that interact with the widgets.
 */

/*
 * 	called by math routines to write to the liquid crystal display.
 */
void draw(string)
    char	*string;
{
    Arg	args[1];

    XtSetArg(args[0], XtNlabel, string);
    XtSetValues(LCD, args, ONE);
}
/*
 *	called by math routines to turn on and off the display indicators.
 */
void setflag(indicator, on)
    int		indicator;
    Boolean	on;
{
    if (on) XtMapWidget(ind[indicator]);
    else XtUnmapWidget(ind[indicator]);
}

/*
 *	ring the bell.
 */
void ringbell()
{
    XBell(dpy, 0);
}

/*
 *	die.
 */
void Quit()
{
    extern void exit();
    XtDestroyApplicationContext(xtcontext);
    exit(0);
}

/*  
 *	recite and die.
 */
void Syntax(argc, argv)	
    int		argc;
    char	**argv;
{
    register int i;
    extern void exit();
    (void) fprintf(stderr, "%s: unknown options:", argv[0]);
    for (i=1; i <argc; i++)
	(void) fprintf(stderr, " %s", argv[i]);
    (void) fprintf(stderr, "\n\n");
    (void) fprintf(stderr, "Usage:  %s", argv[0]);
    for (i=0; i < XtNumber(Options); i++)
	(void) fprintf(stderr, " [%s]", Options[i].option);
    (void) fprintf(stderr, "\n");
    XtDestroyApplicationContext(xtcontext);
    exit(1);
}

/*    
 * I use actions on the toggle widget to support selections.  This
 * means that the user may not do a partial selection of the number
 * displayed in the `liquid crystal display.'  Copying numbers into
 * the calculator is also not supported.  So all you can do is copy
 * the entire number from the calculator display.
 */

/*ARGSUSED*/
Boolean convert(w, selection, target, type, value, length, format)
    Widget	w;
    Atom	*selection;
    Atom	*target;
    Atom	*type;
    XtPointer	*value;
    unsigned long	*length;
    int		*format;
{
    if (*target == XA_STRING)
    {
	*type = XA_STRING;
	*length = strlen(dispstr);
	strncpy(selstr, dispstr, (int)(*length));
	*value = selstr;
	*format = 8;
	return True;
    }
    return False;
}

/*
 * called when xcalc loses ownership of the selection.
 */
/*ARGSUSED*/
void lose(w, selection)
    Widget	w;
    Atom	*selection;
{
    XawToggleUnsetCurrent(LCD);
}

/*
 * called when some other client got the selection.
 */
/*ARGSUSED*/
void done(w, selection, target)
    Widget	w;
    Atom	*selection;
    Atom	*target;
{
    selstr[0] = '\0';
}

/*
 * called by the selection() action routine, in response to user action.
 */
void do_select(time)
    Time	time;
{
    Boolean	state;
    Arg		args[1];

    XtSetArg(args[0], XtNstate, &state);
    XtGetValues(LCD, args, 1);

    if (state)
        XtOwnSelection(LCD, XA_PRIMARY, time, convert, lose, done);
    else
	selstr[0] = '\0';
}
