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

/*
 * Most of the code here is based on the xvidtune code.
 */

#include "vidmode.h"
#include <X11/Xaw/Command.h>
#include <X11/Xaw/Form.h>
#include <X11/Xaw/Label.h>
#include <X11/Xaw/MenuButton.h>
#include <X11/Xaw/Repeater.h>
#include <X11/Shell.h>
#include <X11/Xaw/AsciiText.h>
#include <X11/Xaw/Dialog.h>
#include <X11/Xaw/SimpleMenP.h>
#include <X11/Xaw/SmeBSB.h>
#include <X11/Xaw/Toggle.h>
#include "xf86config.h"

#define V_FLAG_MASK	0x1FF
#define V_PHSYNC	0x001 
#define V_NHSYNC	0x002
#define V_PVSYNC	0x004
#define V_NVSYNC	0x008
#define V_INTERLACE	0x010 
#define V_DBLSCAN	0x020
#define V_CSYNC		0x040
#define V_PCSYNC	0x080
#define V_NCSYNC	0x100

#define	LEFT		0
#define	RIGHT		1
#define	UP		2
#define	DOWN		3
#define	WIDER		4
#define	TALLER		5
#define	NARROWER	6
#define	SHORTER		7

#define	HDISPLAY	0
#define VDISPLAY	1
#define HSYNCSTART	2
#define HSYNCEND	3
#define HTOTAL		4
#define VSYNCSTART	5
#define VSYNCEND	6
#define VTOTAL		7
#define FLAGS		8
#define CLOCK		9
#define HSYNC		10
#define VSYNC		11

#define MINMAJOR	 2
#define MINMINOR	 0

/*
 * Types
 */
typedef struct {
    char *ident;
    XF86VidModeModeInfo info;
} xf86cfgVesaModeInfo;

/*
 * Prototypes
 */
static Bool GetModeLine(Bool);
static void StartAdjustMonitorCallback(Widget, XtPointer, XtPointer);
static void AdjustMonitorCallback(Widget, XtPointer, XtPointer);
static void EndAdjustMonitorCallback(Widget, XtPointer, XtPointer);
static void SetLabel(int, int);
static void UpdateSyncRates(Bool);
static int VidmodeError(Display*, XErrorEvent*);
static void CleanUp(Display*);
static void ApplyCallback(Widget, XtPointer, XtPointer);
static void AutoCallback(Widget, XtPointer, XtPointer);
static void RestoreCallback(Widget, XtPointer, XtPointer);
static void SelectCallback(Widget, XtPointer, XtPointer);
static void SelectMonitorCallback(Widget, XtPointer, XtPointer);
static void SwitchCallback(Widget, XtPointer, XtPointer);
static void SetLabels(void);
static void UpdateCallback(Widget, XtPointer, XtPointer);
static void ChangeScreenCallback(Widget, XtPointer, XtPointer);
static void SetLabelAndModeline(void);
static void AddVesaModeCallback(Widget, XtPointer, XtPointer);
static void GetModes(void);
static void AddModeCallback(Widget, XtPointer, XtPointer);
static void TestCallback(Widget, XtPointer, XtPointer);
static void TestTimeout(XtPointer, XtIntervalId*);
static void StopTestCallback(Widget, XtPointer, XtPointer);
static int ForceAddMode(void);
static int AddMode(void);
/*
 * Initialization
 */
extern Widget work;
Widget vtune;
static Widget apply, automatic, restore, mode, menu, screenb, screenp;
static Bool autoflag;
static xf86cfgVidmode *vidtune;
static XF86VidModeModeLine modeline, orig_modeline;
static int dot_clock, hsync_rate, vsync_rate, hitError;
static int screenno;
static int (*XtErrorFunc)(Display*, XErrorEvent*);
static Widget values[VSYNC + 1], repeater, monitor,
	      monitorb, add, text, vesap, forceshell, testshell, addshell;
static int MajorVersion, MinorVersion, EventBase, ErrorBase;
static XtIntervalId timeout;

/* The information bellow is extracted from
 * xc/programs/Xserver/hw/xfree86/etc/vesamodes
 * If that file is changed, please update the table bellow also. Or even
 * better, write a script to generate the table.
 */
static xf86cfgVesaModeInfo vesamodes[] = {
    {
	"640x350 @ 85Hz (VESA) hsync: 37.9kHz",
	{
	    31500, 640, 672, 736, 832, 0, 350, 382, 385, 445,
	    V_PHSYNC | V_NVSYNC
	}
    },
    {
	"640x400 @ 85Hz (VESA) hsync: 37.9kHz",
	{
	    31500, 640, 672, 736, 832, 0, 400, 401, 404, 445,
	    V_NHSYNC | V_PVSYNC
	}
    },
    {
	"720x400 @ 85Hz (VESA) hsync: 37.9kHz",
	{
	    35500, 720, 756, 828, 936, 0, 400, 401, 404, 446,
	    V_NHSYNC | V_PVSYNC
	}
    },
    {
	"640x480 @ 60Hz (Industry standard) hsync: 31.5kHz",
	{
	    25200, 640, 656, 752, 800, 0, 480, 490, 492, 525,
	    V_NHSYNC | V_NVSYNC
	}
    },
    {
	"640x480 @ 72Hz (VESA) hsync: 37.9kHz",
	{
	    31500, 640, 664, 704, 832, 0, 480, 489, 491, 520,
	    V_NHSYNC | V_NVSYNC
	}
    },
    {
	"640x480 @ 75Hz (VESA) hsync: 37.5kHz",
	{
	    31500, 640, 656, 720, 840, 0, 480, 481, 484, 500,
	    V_NHSYNC | V_NVSYNC
	}
    },
    {
	"640x480 @ 85Hz (VESA) hsync: 43.3kHz",
	{
	    36000, 640, 696, 752, 832, 0, 480, 481, 484, 509,
	    V_NHSYNC | V_NVSYNC
	}
    },
    {
	"800x600 @ 56Hz (VESA) hsync: 35.2kHz",
	{
	    36000, 800, 824, 896, 1024, 0, 600, 601, 603, 625,
	    V_PHSYNC | V_PVSYNC
	}
    },
    {
	"800x600 @ 60Hz (VESA) hsync: 37.9kHz",
	{
	    400000, 800, 840, 968, 1056, 0, 600, 601, 605, 628,
	    V_PHSYNC | V_PVSYNC
	}
    },
    {
	"800x600 @ 72Hz (VESA) hsync: 48.1kHz",
	{
	    50000, 800, 856, 976, 1040, 0, 600, 637, 643, 666,
	    V_PHSYNC | V_PVSYNC
	}
    },
    {
	"800x600 @ 75Hz (VESA) hsync: 46.9kHz",
	{
	    49500, 800, 816, 896, 1056, 0, 600, 601, 604, 625,
	    V_PHSYNC | V_PVSYNC
	}
    },
    {
	"800x600 @ 85Hz (VESA) hsync: 53.7kHz",
	{
	    563000, 800, 832, 896, 1048, 0, 600, 601, 604, 631,
	    V_PHSYNC | V_PVSYNC
	}
    },
    {
	"1024x768i @ 43Hz (industry standard) hsync: 35.5kHz",
	{
	    44900, 1024, 1032, 1208, 1264, 0, 768, 768, 776, 817,
	    V_PHSYNC | V_PVSYNC | V_INTERLACE
	}
    },
    {
	"1024x768 @ 60Hz (VESA) hsync: 48.4kHz",
	{
	    65000, 1024, 1048, 1184, 1344, 0, 768, 771, 777, 806,
	    V_NHSYNC | V_NVSYNC
	}
    },
    {
	"1024x768 @ 70Hz (VESA) hsync: 56.5kHz",
	{
	    75000, 1024, 1048, 1184, 1328, 0, 768, 771, 777, 806,
	    V_NHSYNC | V_NVSYNC
	}
    },
    {
	"1024x768 @ 75Hz (VESA) hsync: 60.0kHz",
	{
	    78800, 1024, 1040, 1136, 1312, 0, 768, 769, 772, 800,
	    V_PHSYNC | V_PVSYNC
	}
    },
    {
	"1024x768 @ 85Hz (VESA) hsync: 68.7kHz",
	{
	    94500, 1024, 1072, 1168, 1376, 0, 768, 769, 772, 808,
	    V_PHSYNC | V_PVSYNC
	}
    },
    {
	"1152x864 @ 75Hz (VESA) hsync: 67.5kHz",
	{
	    108000, 1152, 1216, 1344, 1600, 0, 864, 865, 868, 900,
	    V_PHSYNC | V_PVSYNC
	}
    },
    {
	"1280x960 @ 60Hz (VESA) hsync: 60.0kHz",
	{
	    108000, 1280, 1376, 1488, 1800, 0, 960, 961, 964, 1000,
	    V_PHSYNC | V_PVSYNC
	}
    },
    {
	"1280x960 @ 85Hz (VESA) hsync: 85.9kHz",
	{
	    148500, 1280, 1344, 1504, 1728, 0, 960, 961, 964, 1011,
	    V_PHSYNC | V_PVSYNC
	}
    },
    {
	"1280x1024 @ 60Hz (VESA) hsync: 64.0kHz",
	{
	    108000, 1280, 1328, 1440, 1688, 0, 1024, 1025, 1028, 1066,
	    V_PHSYNC | V_PVSYNC
	}
    },
    {
	"1280x1024 @ 75Hz (VESA) hsync: 80.0kHz",
	{
	    135000, 1280, 1296, 1440, 1688, 0, 1024, 1025, 1028, 1066,
	    V_PHSYNC | V_PVSYNC
	}
    },
    {
	"1280x1024 @ 85Hz (VESA) hsync: 91.1kHz",
	{
	    157500, 1280, 1344, 1504, 1728, 0, 1024, 1025, 1028, 1072,
	    V_PHSYNC | V_PVSYNC
	}
    },
    {
	"1600x1200 @ 60Hz (VESA) hsync: 75.0kHz",
	{
	    162000, 1600, 1664, 1856, 2160, 0, 1200, 1201, 1204, 1250,
	    V_PHSYNC | V_PVSYNC
	}
    },
    {
	"1600x1200 @ 65Hz (VESA) hsync: 81.3kHz",
	{
	    175500, 1600, 1664, 1856, 2160, 0, 1200, 1201, 1204, 1250,
	    V_PHSYNC | V_PVSYNC
	}
    },
    {
	"1600x1200 @ 70Hz (VESA) hsync: 87.5kHz",
	{
	    189000, 1600, 1664, 1856, 2160, 0, 1200, 1201, 1204, 1250,
	    V_PHSYNC | V_PVSYNC
	}
    },
    {
	"1600x1200 @ 75Hz (VESA) hsync: 93.8kHz",
	{
	    202500, 1600, 1664, 1856, 2160, 0, 1200, 1201, 1204, 1250,
	    V_PHSYNC | V_PVSYNC
	}
    },
    {
	"1600x1200 @ 85Hz (VESA) hsync: 106.3kHz",
	{
	    229500, 1600, 1664, 1856, 2160, 0, 1200, 1201, 1204, 1250,
	    V_PHSYNC | V_PVSYNC
	}
    },
    {
	"1792x1344 @ 60Hz (VESA) hsync: 83.6kHz",
	{
	    204800, 1792, 1920, 2120, 2448, 0, 1344, 1345, 1348, 1394,
	    V_NHSYNC | V_PVSYNC
	}
    },
    {
	"1792x1344 @ 75Hz (VESA) hsync: 106.3kHz",
	{
	    261000, 1792, 1888, 2104, 2456, 0, 1344, 1345, 1348, 1417,
	    V_NHSYNC | V_PVSYNC
	}
    },
    {
	"1856x1392 @ 60Hz (VESA) hsync: 86.3kHz",
	{
	    218300, 1856, 1952, 2176, 2528, 0, 1392, 1393, 1396, 1439,
	    V_NHSYNC | V_PVSYNC
	}
    },
    {
	"1856x1392 @ 75Hz (VESA) hsync: 112.5kHz",
	{
	    288000, 1856, 1984, 2208, 2560, 0, 1392, 1393, 1396, 1500,
	    V_NHSYNC | V_PVSYNC
	}
    },
    {
	"1920x1440 @ 60Hz (VESA) hsync: 90.0kHz",
	{
	    234000, 1920, 2048, 2256, 2600, 0, 1440, 1441, 1444, 1500,
	    V_NHSYNC | V_PVSYNC
	}
    },
    {
	"1920x1440 @ 75Hz (VESA) hsync: 112.5kHz",
	{
	    297000, 1920, 2064, 2288, 2640, 0, 1440, 1441, 1444, 1500,
	    V_NHSYNC | V_PVSYNC
	}
    },
};

/*
 * Implementation
 */
Bool
VideoModeInitialize(void)
{
    Widget form;
    char dispstr[128], *ptr, *tmp;

    static char *names[] = {
	NULL,
	NULL,
	"hsyncstart",
	"hsyncend",
	"htotal",
	"vsyncstart",
	"vsyncend",
	"vtotal",
	"flags",
	"clock",
	"hsync",
	"vsync",
    };
    static char *vnames[] = {
	NULL,
	NULL,
	"v-hsyncstart",
	"v-hsyncend",
	"v-htotal",
	"v-vsyncstart",
	"v-vsyncend",
	"v-vtotal",
	"v-flags",
	"v-clock",
	"v-hsync",
	"v-vsync",
    };
    Widget rep;
    int i;

    if (!XF86VidModeQueryVersion(XtDisplay(toplevel),
				 &MajorVersion, &MinorVersion)) {
	fprintf(stderr, "Unable to query video extension version\n");
	return (False);
    }
    else if (!XF86VidModeQueryExtension(XtDisplay(toplevel),
					&EventBase, &ErrorBase)) {
	fprintf(stderr, "Unable to query video extension information\n");
	return (False);
    }
    else if (MajorVersion < MINMAJOR ||
	(MajorVersion == MINMAJOR && MinorVersion < MINMINOR)) {
	fprintf(stderr,
		"Xserver is running an old XFree86-VidModeExtension version"
		" (%d.%d)\n", MajorVersion, MinorVersion);
	fprintf(stderr, "Minimum required version is %d.%d\n",
		MINMAJOR, MINMINOR);
	return (False);
    }
    else
	InitializeVidmodes();

    vtune = XtCreateWidget("vidtune", formWidgetClass,
			   work, NULL, 0);

    (void) XtVaCreateManagedWidget("vesaB", menuButtonWidgetClass, vtune,
				    XtNmenuName, "vesaP", NULL);
    vesap = XtCreatePopupShell("vesaP", simpleMenuWidgetClass, vtune, NULL, 0);
    for (i = 0; i < sizeof(vesamodes) / sizeof(vesamodes[0]); i++) {
	rep = XtCreateManagedWidget(vesamodes[i].ident, smeBSBObjectClass,
				    vesap, NULL, 0);
	XtAddCallback(rep, XtNcallback, AddVesaModeCallback,
		      (XtPointer)&vesamodes[i]);
    }

    rep = XtCreateManagedWidget("prev", commandWidgetClass, vtune, NULL, 0);
    XtAddCallback(rep, XtNcallback, SwitchCallback, (XtPointer)-1);
    mode = XtCreateManagedWidget("mode", menuButtonWidgetClass, vtune, NULL, 0);
    rep = XtCreateManagedWidget("next", commandWidgetClass, vtune, NULL, 0);
    XtAddCallback(rep, XtNcallback, SwitchCallback, (XtPointer)1);

    screenp = XtCreatePopupShell("screenP", simpleMenuWidgetClass, vtune,
				 NULL, 0);

    XmuSnprintf(dispstr, sizeof(dispstr), "%s",
		DisplayString(XtDisplay(toplevel)));
    ptr = strrchr(dispstr, '.');
    tmp = strrchr(dispstr, ':');
    if (tmp != NULL && ptr != NULL && ptr > tmp)
	*ptr = '\0';

    for (i = 0; i < ScreenCount(XtDisplay(toplevel)); i++) {
	char name[128];

	XmuSnprintf(name, sizeof(name), "%s.%d", dispstr, i);
	rep = XtCreateManagedWidget(name, smeBSBObjectClass, screenp,
				    NULL, 0);
	XtAddCallback(rep, XtNcallback, ChangeScreenCallback,
	    (XtPointer)(long)i);
	if (i == 0) {
	    screenb = XtVaCreateManagedWidget("screenB", menuButtonWidgetClass,
					      vtune,
					      XtNmenuName, "screenP",
					      XtNlabel, name,
					      NULL);
	}
    }
    XtRealizeWidget(screenp);

    rep = XtCreateManagedWidget("up", repeaterWidgetClass,
				vtune, NULL, 0);
    XtAddCallback(rep, XtNstartCallback, StartAdjustMonitorCallback, NULL);
    XtAddCallback(rep, XtNcallback,
		  AdjustMonitorCallback, (XtPointer)UP);
    XtAddCallback(rep, XtNstopCallback, EndAdjustMonitorCallback, NULL);
    rep = XtCreateManagedWidget("left", repeaterWidgetClass,
				vtune, NULL, 0);
    XtAddCallback(rep, XtNstartCallback, StartAdjustMonitorCallback, NULL);
    XtAddCallback(rep, XtNcallback,
		  AdjustMonitorCallback, (XtPointer)LEFT);
    XtAddCallback(rep, XtNstopCallback, EndAdjustMonitorCallback, NULL);
    XtCreateManagedWidget("monitor", simpleWidgetClass, vtune, NULL, 0);
    rep = XtCreateManagedWidget("right", repeaterWidgetClass,
				vtune, NULL, 0);
    XtAddCallback(rep, XtNstartCallback, StartAdjustMonitorCallback, NULL);
    XtAddCallback(rep, XtNcallback,
		  AdjustMonitorCallback, (XtPointer)RIGHT);
    XtAddCallback(rep, XtNstopCallback, EndAdjustMonitorCallback, NULL);
    rep = XtCreateManagedWidget("down", repeaterWidgetClass,
				vtune, NULL, 0);
    XtAddCallback(rep, XtNstartCallback, StartAdjustMonitorCallback, NULL);
    XtAddCallback(rep, XtNcallback,
		  AdjustMonitorCallback, (XtPointer)DOWN);
    XtAddCallback(rep, XtNstopCallback, EndAdjustMonitorCallback, NULL);
    rep = XtCreateManagedWidget("wider", repeaterWidgetClass,
				vtune, NULL, 0);
    XtAddCallback(rep, XtNstartCallback, StartAdjustMonitorCallback, NULL);
    XtAddCallback(rep, XtNcallback,
		  AdjustMonitorCallback, (XtPointer)WIDER);
    XtAddCallback(rep, XtNstopCallback, EndAdjustMonitorCallback, NULL);
    rep = XtCreateManagedWidget("narrower", repeaterWidgetClass,
				vtune, NULL, 0);
    XtAddCallback(rep, XtNstartCallback, StartAdjustMonitorCallback, NULL);
    XtAddCallback(rep, XtNcallback,
		  AdjustMonitorCallback, (XtPointer)NARROWER);
    XtAddCallback(rep, XtNstopCallback, EndAdjustMonitorCallback, NULL);
    rep = XtCreateManagedWidget("shorter", repeaterWidgetClass,
				vtune, NULL, 0);
    XtAddCallback(rep, XtNstartCallback, StartAdjustMonitorCallback, NULL);
    XtAddCallback(rep, XtNcallback,
		  AdjustMonitorCallback, (XtPointer)SHORTER);
    XtAddCallback(rep, XtNstopCallback, EndAdjustMonitorCallback, NULL);
    rep = XtCreateManagedWidget("taller", repeaterWidgetClass,
				vtune, NULL, 0);
    XtAddCallback(rep, XtNstartCallback, StartAdjustMonitorCallback, NULL);
    XtAddCallback(rep, XtNcallback,
		  AdjustMonitorCallback, (XtPointer)TALLER);
    XtAddCallback(rep, XtNstopCallback, EndAdjustMonitorCallback, NULL);

    automatic = XtCreateManagedWidget("auto", toggleWidgetClass, vtune, NULL, 0);
    XtAddCallback(automatic, XtNcallback, AutoCallback, NULL);
    apply = XtCreateManagedWidget("apply", commandWidgetClass, vtune, NULL, 0);
    XtAddCallback(apply, XtNcallback, ApplyCallback, NULL);
    restore = XtCreateManagedWidget("restore", commandWidgetClass, vtune, NULL, 0);
    XtAddCallback(restore, XtNcallback, RestoreCallback, NULL);
    rep = XtCreateManagedWidget("update", commandWidgetClass, vtune, NULL, 0);
    XtAddCallback(rep, XtNcallback, UpdateCallback, NULL);
    rep = XtCreateManagedWidget("test", commandWidgetClass, vtune, NULL, 0);
    XtAddCallback(rep, XtNcallback, TestCallback, NULL);

    form = XtCreateManagedWidget("form", formWidgetClass, vtune, NULL, 0);
    for (i = 2; i < VSYNC + 1; i++) {
	(void) XtCreateManagedWidget(names[i], labelWidgetClass,
					  form, NULL, 0);
	values[i] = XtCreateManagedWidget(vnames[i], labelWidgetClass,
					  form, NULL, 0);
    }

    add = XtCreateManagedWidget("add", commandWidgetClass, vtune, NULL, 0);
    XtAddCallback(add, XtNcallback, AddModeCallback, NULL);
    XtCreateManagedWidget("addto", labelWidgetClass, vtune, NULL, 0);
    monitorb = XtCreateManagedWidget("ident", menuButtonWidgetClass, vtune,
				     NULL, 0);
    XtCreateManagedWidget("as", labelWidgetClass, vtune, NULL, 0);
    text = XtVaCreateManagedWidget("text", asciiTextWidgetClass, vtune,
				   XtNeditType, XawtextEdit, NULL);

    XtRealizeWidget(vtune);

    return (True);
}

void
InitializeVidmodes(void)
{
    int i;
    Display *display = XtDisplay(toplevel);

    computer.num_vidmodes = ScreenCount(display);
    computer.vidmodes = (xf86cfgVidmode**)
	XtMalloc(sizeof(xf86cfgVidmode*) * computer.num_vidmodes);
    for (i = 0; i < computer.num_vidmodes; i++) {

	computer.vidmodes[i] = (xf86cfgVidmode*)
	    XtCalloc(1, sizeof(xf86cfgVidmode));
	computer.vidmodes[i]->screen = i;
    }
}

void
VideoModeConfigureStart(void)
{
    vidtune = computer.vidmodes[screenno];

    XtSetSensitive(vtune, vidtune != NULL);
    if (!XtIsManaged(vtune))
	XtManageChild(vtune);
    else
	XtMapWidget(vtune);
    if (vidtune != NULL) {
	Arg args[1];
	Boolean state;
	XF86ConfMonitorPtr mon;
	static char menuName[16];
	static int menuN;

	XtErrorFunc = XSetErrorHandler(VidmodeError);
	XF86VidModeLockModeSwitch(XtDisplay(toplevel), vidtune->screen, True);
	GetModeLine(True);
	GetModes();

	SetLabels();
	XtSetArg(args[0], XtNstate, &state);
	XtGetValues(automatic, args, 1);
	XtSetSensitive(apply, !state);
	autoflag = state;

	if (monitor)
	    XtDestroyWidget(monitor);
	XmuSnprintf(menuName, sizeof(menuName), "menuP%d", menuN);
	menuN = !menuN;
	monitor = XtCreatePopupShell(menuName, simpleMenuWidgetClass,
				     vtune, NULL, 0);
	XtVaSetValues(monitorb, XtNmenuName, menuName, NULL);

	mon = XF86Config->conf_monitor_lst;
	while (mon != NULL) {
	    Widget sme = XtCreateManagedWidget(mon->mon_identifier,
					       smeBSBObjectClass,
					       monitor, NULL, 0);
	    XtAddCallback(sme, XtNcallback,
			  SelectMonitorCallback, (XtPointer)mon);

	    /* guess the monitor at a given screen and/or
	     * updates configuration if a monitor was removed from the
	     * configuration.
	     */
	    if (XF86Config->conf_layout_lst) {
		XF86ConfAdjacencyPtr adj = XF86Config->conf_layout_lst->
		    lay_adjacency_lst;

		while (adj != NULL) {
		    if (adj->adj_screen != NULL) {
			if (adj->adj_screen->scrn_monitor == mon &&
			    adj->adj_scrnum >= 0 &&
			    adj->adj_scrnum < ScreenCount(XtDisplay(toplevel))) {
			    if (computer.vidmodes[adj->adj_scrnum]->monitor ==
				NULL || computer.vidmodes[adj->adj_scrnum]->
				monitor == adj->adj_screen->scrn_monitor) {
				computer.vidmodes[adj->adj_scrnum]->monitor =
				    adj->adj_screen->scrn_monitor;
				break;
			    }
			    else
				computer.vidmodes[adj->adj_scrnum]->monitor =
				    NULL;
			}
		    }
		    adj = (XF86ConfAdjacencyPtr)(adj->list.next);
		}
	    }
	    mon = (XF86ConfMonitorPtr)(mon->list.next);
	}
	SetLabelAndModeline();
    }
}

void
VideoModeConfigureEnd(void)
{
    XtUnmapWidget(vtune);
    if (vidtune != NULL) {
	XF86VidModeLockModeSwitch(XtDisplay(toplevel), vidtune->screen, False);
	XSetErrorHandler(XtErrorFunc);
    }
    vidtune = NULL;
}

static void
SetLabelAndModeline(void)
{
    if (vidtune->monitor != NULL) {
	char string[32];

	XtVaSetValues(monitorb, XtNlabel,
		      vidtune->monitor->mon_identifier, NULL);
	XtSetSensitive(add, True);

	if (modeline.htotal && modeline.vtotal)
	    XmuSnprintf(string, sizeof(string), "%dx%d@%d",
			modeline.hdisplay, modeline.vdisplay,
			(int)((double)dot_clock / (double)modeline.htotal * 1000.0 /
			(double)modeline.vtotal));
	else
	    XmuSnprintf(string, sizeof(string), "%dx%d",
			modeline.hdisplay, modeline.vdisplay);
	XtVaSetValues(text, XtNstring, string, NULL);
    }
    else {
	XtVaSetValues(monitorb, XtNlabel, "", NULL);
	XtSetSensitive(add, False);
	XtVaSetValues(text, XtNstring, "", NULL);
    }
}

/*ARGSUSED*/
void
VidmodeRestoreAction(Widget w, XEvent *event,
		     String *params, Cardinal *num_params)
{
    if (vidtune != NULL) {
	if (timeout != 0)
	    StopTestCallback(w, NULL, NULL);
	else
	    RestoreCallback(w, NULL, NULL);
    }
}

static void
UpdateSyncRates(Bool update)
{
    if (modeline.htotal && modeline.vtotal) {
	hsync_rate = (dot_clock * 1000) / modeline.htotal;
	vsync_rate = (hsync_rate * 1000) / modeline.vtotal;
	if (modeline.flags & V_INTERLACE)
	    vsync_rate *= 2;
	else if (modeline.flags & V_DBLSCAN)
	    vsync_rate /= 2;
	if (update) {
	    SetLabel(HSYNC, hsync_rate);
	    SetLabel(VSYNC, vsync_rate);
	}
    }
}

static void
SetLabel(int ident, int value)
{
    Arg args[1];
    char label[256];

    if (ident == FLAGS) {
	int len = 0;

	*label = '\0';
	if (value & V_PHSYNC)
	    len += XmuSnprintf(label, sizeof(label), "%s", "+hsync");
	if (modeline.flags & V_NHSYNC)
	    len += XmuSnprintf(label + len, sizeof(label), "%s%s",
			       len ? " " : "", "-hsync");
	if (value & V_PVSYNC)
	    len += XmuSnprintf(label + len, sizeof(label), "%s%s",
			       len ? " " : "", "+vsync");
	if (value & V_NVSYNC)
	    len += XmuSnprintf(label + len, sizeof(label), "%s%s",
			       len ? " " : "", "-vsync");
	if (value & V_INTERLACE)
	    len += XmuSnprintf(label + len, sizeof(label), "%s%s",
			       len ? " " : "", "interlace");
	if (value & V_CSYNC)
	    len += XmuSnprintf(label + len, sizeof(label), "%s%s",
			       len ? " " : "", "composite");
	if (value & V_PCSYNC)
	    len += XmuSnprintf(label + len, sizeof(label), "%s%s",
			       len ? " " : "", "+csync");
	if (value & V_NCSYNC)
	    len += XmuSnprintf(label + len, sizeof(label), "%s%s",
			       len ? " " : "", "-csync");
	if (value & V_DBLSCAN)
	    len += XmuSnprintf(label + len, sizeof(label), "%s%s",
			       len ? " " : "", "doublescan");

    }
    else if (ident == CLOCK || ident == HSYNC || ident == VSYNC)
	XmuSnprintf(label, sizeof(label), "%6.2f", (float)value / 1000.0);
    else
	XmuSnprintf(label, sizeof(label), "%d", value);

    XtSetArg(args[0], XtNlabel, label);
    XtSetValues(values[ident], args, 1);
}

/*ARGSUSED*/
static void
StartAdjustMonitorCallback(Widget w, XtPointer client_data, XtPointer call_data)
{
    repeater = w;
}

static void
AdjustMonitorCallback(Widget w, XtPointer client_data, XtPointer call_data)
{
    if (repeater != w)
	return;
    switch ((long)client_data) {
	case LEFT:
	    if (modeline.hsyncend + 4 < modeline.htotal) {
		modeline.hsyncstart += 4;
		modeline.hsyncend += 4;
		SetLabel(HSYNCSTART, modeline.hsyncstart);
		SetLabel(HSYNCEND, modeline.hsyncend);
	    }
	    else
		XBell(XtDisplay(w), 80);
	    break;
	case RIGHT:
	    if (modeline.hsyncstart - 4 > modeline.hdisplay) {
		modeline.hsyncstart -= 4;
		modeline.hsyncend -= 4;
		SetLabel(HSYNCSTART, modeline.hsyncstart);
		SetLabel(HSYNCEND, modeline.hsyncend);
	    }
	    else
		XBell(XtDisplay(w), 80);
	    break;
	case NARROWER:
	    modeline.htotal += 4;
	    SetLabel(HTOTAL, modeline.htotal);
	    UpdateSyncRates(True);
	    break;
	case WIDER:
	    if (modeline.htotal - 4 > modeline.hsyncend) {
		modeline.htotal -= 4;
		SetLabel(HTOTAL, modeline.htotal);
		UpdateSyncRates(True);
	    }
	    else
		XBell(XtDisplay(w), 80);
	    break;
	case UP:
	    if (modeline.vsyncend + 4 < modeline.vtotal) {
		modeline.vsyncstart += 4;
		modeline.vsyncend += 4;
		SetLabel(VSYNCSTART, modeline.vsyncstart);
		SetLabel(VSYNCEND, modeline.vsyncend);
	    }
	    else
		XBell(XtDisplay(w), 80);
	    break;
	case DOWN:
	    if (modeline.vsyncstart - 4 > modeline.vdisplay) {
		modeline.vsyncstart -= 4;
		modeline.vsyncend -= 4;
		SetLabel(VSYNCSTART, modeline.vsyncstart);
		SetLabel(VSYNCEND, modeline.vsyncend);
	    }
	    else
		XBell(XtDisplay(w), 80);
	    break;
	case SHORTER:
	    modeline.vtotal += 4;
	    SetLabel(VTOTAL, modeline.vtotal);
	    UpdateSyncRates(True);
	    break;
	case TALLER:
	    if (modeline.vtotal - 4 >  modeline.vsyncend) {
		modeline.vtotal -= 4;
		SetLabel(VTOTAL, modeline.vtotal);
		UpdateSyncRates(True);
	    }
	    else
		XBell(XtDisplay(w), 80);
	    break;
    }

    if (autoflag)
	ApplyCallback(w, call_data, client_data);
}

/*ARGSUSED*/
static void
EndAdjustMonitorCallback(Widget w, XtPointer client_data, XtPointer call_data)
{
    repeater = NULL;
}

static Bool
GetModeLine(Bool save)
{
    if (XF86VidModeGetModeLine(XtDisplay(toplevel), vidtune->screen,
			       &dot_clock, &modeline)) {
	if (save)
	    memcpy(&orig_modeline, &modeline, sizeof(XF86VidModeModeLine));
	UpdateSyncRates(False);
	return (True);
    }

    return (False);
}

static void
CleanUp(Display *display)
{
    /* Make sure mode switching is not locked out at exit */
    XF86VidModeLockModeSwitch(display, vidtune->screen, False);
    XFlush(display);
}

static int
VidmodeError(Display *display, XErrorEvent *error)
{
    if ((error->error_code >= ErrorBase &&
	 error->error_code < ErrorBase + XF86VidModeNumberErrors) ||
	 error->error_code == BadValue) {
	 hitError = 1;
    }
    else {
	CleanUp(display);
	if (XtErrorFunc) 
	    (*XtErrorFunc)(display, error);
    }
    return (0);
}

/*ARGSUSED*/
static void
ApplyCallback(Widget w, XtPointer call_data, XtPointer client_data)
{
    hitError = 0;
    XF86VidModeModModeLine(XtDisplay(w), vidtune->screen, &modeline);
    XSync(XtDisplay(w), False);
    if (hitError) {
	if (repeater != NULL) {
	    XtCallActionProc(repeater, "unset", NULL, NULL, 0);
	    XtCallActionProc(repeater, "stop", NULL, NULL, 0);
	    repeater = NULL;
	}
	XBell(XtDisplay(w), 80);
	if (timeout)
	    StopTestCallback(w, NULL, NULL);
	GetModeLine(False);
	SetLabels();
    }
}

/*ARGSUSED*/
static void
AutoCallback(Widget w, XtPointer call_data, XtPointer client_data)
{
    autoflag = (Bool)(long)client_data;
    XtSetSensitive(apply, !autoflag);
}

static void
RestoreCallback(Widget w, XtPointer call_data, XtPointer client_data)
{
    memcpy(&modeline, &orig_modeline, sizeof(XF86VidModeModeLine));
    if (autoflag)
	ApplyCallback(w, call_data, client_data);
    SetLabels();
}

static void
SelectCallback(Widget w, XtPointer call_data, XtPointer client_data)
{
    XF86VidModeModeInfo *info = (XF86VidModeModeInfo*)call_data;
    Arg args[1];
    Bool result;

    XF86VidModeLockModeSwitch(XtDisplay(toplevel), vidtune->screen, False);
    result = XF86VidModeSwitchToMode(XtDisplay(toplevel), vidtune->screen, info);
    XF86VidModeLockModeSwitch(XtDisplay(toplevel), vidtune->screen, True);
    if (!result)
	return;

    XtSetArg(args[0], XtNlabel, XtName(w));
    XtSetValues(mode, args, 1);
    UpdateCallback(w, call_data, client_data);
}

static void
SwitchCallback(Widget w, XtPointer call_data, XtPointer client_data)
{
    int direction = (long)call_data;
    Arg args[1];
    Bool result;
    char label[32];

    XF86VidModeLockModeSwitch(XtDisplay(toplevel), vidtune->screen, False);
    result = XF86VidModeSwitchMode(XtDisplay(toplevel), vidtune->screen,
				   direction);
    XF86VidModeLockModeSwitch(XtDisplay(toplevel), vidtune->screen, True);
    if (!result)
	return;

    UpdateCallback(w, call_data, client_data);

    if (modeline.htotal && modeline.vtotal)
	XmuSnprintf(label, sizeof(label), "%dx%d @ %d Hz",
		    modeline.hdisplay, modeline.vdisplay,
		    (int)((double)dot_clock / (double)modeline.htotal * 1000.0 /
		    (double)modeline.vtotal));
    else
	XmuSnprintf(label, sizeof(label), "%dx%d",
		    modeline.hdisplay, modeline.vdisplay);
    XtSetArg(args[0], XtNlabel, label);
    XtSetValues(mode, args, 1);
}

/*ARGSUSED*/
static void
UpdateCallback(Widget w, XtPointer call_data, XtPointer client_data)
{
    GetModeLine(True);
    SetLabels();
    SetLabelAndModeline();
}

static void
SetLabels(void)
{
    SetLabel(HSYNCSTART, modeline.hsyncstart);
    SetLabel(VSYNCSTART, modeline.vsyncstart);
    SetLabel(HSYNCEND, modeline.hsyncend);
    SetLabel(VSYNCEND, modeline.vsyncend);
    SetLabel(HTOTAL, modeline.htotal);
    SetLabel(VTOTAL, modeline.vtotal);
    SetLabel(FLAGS, modeline.flags);
    SetLabel(CLOCK, dot_clock);
    UpdateSyncRates(True);
}

/*ARGSUSED*/
static void
ChangeScreenCallback(Widget w, XtPointer call_data, XtPointer client_data)
{
    Arg args[1];

    screenno = (long)call_data;
    if (screenno > computer.num_vidmodes || screenno < 0 ||
	vidtune == computer.vidmodes[screenno])
	return;

    XF86VidModeLockModeSwitch(XtDisplay(toplevel), vidtune->screen, False);
    vidtune = computer.vidmodes[screenno];
    XF86VidModeLockModeSwitch(XtDisplay(toplevel), vidtune->screen, True);
    UpdateCallback(w, call_data, client_data);
    GetModes();

    XtSetArg(args[0], XtNlabel, XtName(w));
    XtSetValues(screenb, args, 1);

    SetLabelAndModeline();
}

/*ARGSUSED*/
static void
SelectMonitorCallback(Widget w, XtPointer call_data, XtPointer client_data)
{
    vidtune->monitor = (XF86ConfMonitorPtr)(call_data);
    SetLabelAndModeline();
}

/*ARGSUSED*/
static void
AddVesaModeCallback(Widget w, XtPointer call_data, XtPointer client_data)
{
    xf86cfgVesaModeInfo *vesa = (xf86cfgVesaModeInfo*)call_data;
    XF86VidModeModeInfo mode;
    int num_infos = vidtune->num_infos;

    memcpy(&mode, &vesa->info, sizeof(XF86VidModeModeInfo));
    if (XF86VidModeAddModeLine(XtDisplay(toplevel), vidtune->screen,
			       &vesa->info, &mode)) {
	XSync(XtDisplay(toplevel), False);
	GetModes();
    }
    else {
	XBell(XtDisplayOfObject(w), 80);
	return;
    }

    if (vidtune && num_infos == vidtune->num_infos) {
	/* XF86VidModeAddModeLine returned True, but no modeline was added */
	XBell(XtDisplayOfObject(w), 80);
	if (vidtune->monitor && AddMode()) {
	    XF86ConfModeLinePtr mode;
	    char label[256], *ptr, *str;

	    XmuSnprintf(label, sizeof(label), "%s", vesa->ident);

	    /* format mode name to not have spaces */
	    ptr = strchr(label, ')');
	    if (ptr)
		*++ptr = '\0';
	    ptr = str = label;
	    while (*ptr) {
		if (*ptr != ' ')
		    *str++ = *ptr;
		++ptr;
	    }
	    *str = '\0';

	    if (xf86findModeLine(label, vidtune->monitor->mon_modeline_lst)
		!= NULL && !ForceAddMode())
		return;

	    mode = (XF86ConfModeLinePtr)XtCalloc(1, sizeof(XF86ConfModeLineRec));
	    mode->ml_identifier = XtNewString(label);
	    mode->ml_clock = vesa->info.dotclock;
	    mode->ml_hdisplay = vesa->info.hdisplay;
	    mode->ml_hsyncstart = vesa->info.hsyncstart;
	    mode->ml_hsyncend = vesa->info.hsyncend;
	    mode->ml_htotal = vesa->info.htotal;
	    mode->ml_vdisplay = vesa->info.vdisplay;
	    mode->ml_vsyncstart = vesa->info.vsyncstart;
	    mode->ml_vsyncend = vesa->info.vsyncend;
	    mode->ml_vtotal = vesa->info.vtotal;
/*	    mode->ml_vscan = ???;*/
	    mode->ml_flags = vesa->info.flags;
	    mode->ml_hskew = vesa->info.hskew;
	    vidtune->monitor->mon_modeline_lst =
		xf86addModeLine(vidtune->monitor->mon_modeline_lst, mode);
	}
    }
}

static void
GetModes(void)
{
    int i;
    char label[32];
    Arg args[1];
    static char menuName[16];
    static int menuN;

    XFree(vidtune->infos);
    XF86VidModeGetAllModeLines(XtDisplay(toplevel), vidtune->screen,
			       &vidtune->num_infos, &vidtune->infos);

    XmuSnprintf(menuName, sizeof(menuName), "menu%d", menuN);
    menuN = !menuN;
    if (menu)
	XtDestroyWidget(menu);
    menu = XtCreatePopupShell(menuName, simpleMenuWidgetClass, vtune, NULL, 0);
    XtVaSetValues(mode, XtNmenuName, menuName, NULL);
    for (i = 0; i < vidtune->num_infos; i++) {
	Widget sme;

	if ((double)vidtune->infos[i]->htotal &&
	    (double)vidtune->infos[i]->vtotal)
	    XmuSnprintf(label, sizeof(label), "%dx%d @ %d Hz",
			vidtune->infos[i]->hdisplay,
			vidtune->infos[i]->vdisplay,
			(int)((double)vidtune->infos[i]->dotclock /
			(double)vidtune->infos[i]->htotal * 1000.0 /
			(double)vidtune->infos[i]->vtotal));
	else
	    XmuSnprintf(label, sizeof(label), "%dx%d",
			vidtune->infos[i]->hdisplay,
			vidtune->infos[i]->vdisplay);
	sme = XtCreateManagedWidget(label, smeBSBObjectClass, menu, NULL, 0);
	XtAddCallback(sme, XtNcallback, SelectCallback,
		      (XtPointer)vidtune->infos[i]);
    }

    if (modeline.htotal && modeline.vtotal)
	XmuSnprintf(label, sizeof(label), "%dx%d @ %d Hz",
		    modeline.hdisplay, modeline.vdisplay,
		    (int)((double)dot_clock / (double)modeline.htotal * 1000.0 /
		    (double)modeline.vtotal));
    else
	XmuSnprintf(label, sizeof(label), "%dx%d",
		    modeline.hdisplay, modeline.vdisplay);
    XtSetArg(args[0], XtNlabel, label);
    XtSetValues(mode, args, 1);
}

static int do_force, asking_force;

static void
PopdownForce(Widget w, XtPointer user_data, XtPointer call_data)
{
    asking_force = 0;
    XtPopdown(forceshell);
    do_force = (long)user_data;
}

static int
ForceAddMode(void)
{
    if (forceshell == NULL) {
	Widget dialog;

	forceshell = XtCreatePopupShell("force", transientShellWidgetClass,
					toplevel, NULL, 0);
	dialog = XtVaCreateManagedWidget("dialog", dialogWidgetClass,
					 forceshell, XtNvalue, NULL, NULL);
	XawDialogAddButton(dialog, "yes", PopdownForce, (XtPointer)True);
	XawDialogAddButton(dialog, "no", PopdownForce, (XtPointer)False);
	XtRealizeWidget(forceshell);
	XSetWMProtocols(DPY, XtWindow(forceshell), &wm_delete_window, 1);
    }

    asking_force = 1;

    XtPopup(forceshell, XtGrabExclusive);
    while (asking_force)
	XtAppProcessEvent(XtWidgetToApplicationContext(forceshell), XtIMAll);

    return (do_force);
}

static int do_add, asking_add;

static void
PopdownAdd(Widget w, XtPointer user_data, XtPointer call_data)
{
    asking_add = 0;
    XtPopdown(addshell);
    do_add = (long)user_data;
}

void
CancelAddModeAction(Widget w, XEvent *event,
		       String *params, Cardinal *num_params)
{
    if (asking_force)
	PopdownForce(w, (XtPointer)False, NULL);
    else if (asking_add)
	PopdownAdd(w, (XtPointer)False, NULL);
}

static int
AddMode(void)
{
    if (addshell == NULL) {
	Widget dialog;

	addshell = XtCreatePopupShell("addMode", transientShellWidgetClass,
				      toplevel, NULL, 0);
	dialog = XtVaCreateManagedWidget("dialog", dialogWidgetClass,
					 addshell, XtNvalue, NULL, NULL);
	XawDialogAddButton(dialog, "yes", PopdownAdd, (XtPointer)True);
	XawDialogAddButton(dialog, "no", PopdownAdd, (XtPointer)False);
	XtRealizeWidget(addshell);
	XSetWMProtocols(DPY, XtWindow(addshell), &wm_delete_window, 1);
    }

    asking_add = 1;

    XtPopup(addshell, XtGrabExclusive);
    while (asking_add)
	XtAppProcessEvent(XtWidgetToApplicationContext(addshell), XtIMAll);

    return (do_add);
}

/*ARGSUSED*/
static void
AddModeCallback(Widget w, XtPointer call_data, XtPointer client_data)
{
    if (vidtune && vidtune->monitor) {
	char *label;
	Arg args[1];
	XF86ConfModeLinePtr mode;

	XtSetArg(args[0], XtNstring, &label);
	XtGetValues(text, args, 1);
	if (*label == '\0') {
	    XBell(XtDisplay(w), 80);
	    return;
	}	    
	if (xf86findModeLine(label, vidtune->monitor->mon_modeline_lst)
	    != NULL && !ForceAddMode())
	    return;

	mode = (XF86ConfModeLinePtr)XtCalloc(1, sizeof(XF86ConfModeLineRec));
	mode->ml_identifier = XtNewString(label);
	mode->ml_clock = dot_clock;
	mode->ml_hdisplay = modeline.hdisplay;
	mode->ml_hsyncstart = modeline.hsyncstart;
	mode->ml_hsyncend = modeline.hsyncend;
	mode->ml_htotal = modeline.htotal;
	mode->ml_vdisplay = modeline.vdisplay;
	mode->ml_vsyncstart = modeline.vsyncstart;
	mode->ml_vsyncend = modeline.vsyncend;
	mode->ml_vtotal = modeline.vtotal;
/*	mode->ml_vscan = ???;*/
	mode->ml_flags = modeline.flags;
	mode->ml_hskew = modeline.hskew;
	vidtune->monitor->mon_modeline_lst =
	    xf86addModeLine(vidtune->monitor->mon_modeline_lst, mode);
    }
    else
	XBell(XtDisplay(w), 80);
}

/*ARGSUSED*/
static void
StopTestCallback(Widget w, XtPointer call_data, XtPointer client_data)
{
    XtRemoveTimeOut(timeout);
    TestTimeout((XtPointer)w, NULL);
}

/*ARGSUSED*/
void
CancelTestModeAction(Widget w, XEvent *event,
		     String *params, Cardinal *num_params)
{
    StopTestCallback(w, NULL, NULL);
}

static void
TestTimeout(XtPointer client_data, XtIntervalId* id)
{
    XF86VidModeModeLine mode;

    XtPopdown(testshell);
    timeout = 0;
    memcpy(&mode, &modeline, sizeof(XF86VidModeModeLine));
    memcpy(&modeline, &orig_modeline, sizeof(XF86VidModeModeLine));
    ApplyCallback((Widget)client_data, NULL, NULL);
/*    if (hitError == 0)*/
	memcpy(&modeline, &mode, sizeof(XF86VidModeModeLine));
    SetLabels();
}

static void
TestCallback(Widget w, XtPointer call_data, XtPointer client_data)
{
    if (testshell == NULL) {
	Widget dialog;

	testshell = XtCreatePopupShell("test", transientShellWidgetClass,
					toplevel, NULL, 0);
	dialog = XtVaCreateManagedWidget("dialog", dialogWidgetClass,
					 testshell, XtNvalue, NULL, NULL);
	XawDialogAddButton(dialog, "stop", StopTestCallback, NULL);
	XtRealizeWidget(testshell);
	XSetWMProtocols(DPY, XtWindow(testshell), &wm_delete_window, 1);
    }

    XtPopup(testshell, XtGrabExclusive);

    XSync(XtDisplay(toplevel), False);
    timeout = XtAppAddTimeOut(XtWidgetToApplicationContext(w),
    /* the timeout probably shoud be converted to a resource */
			      4000, TestTimeout, (XtPointer)w);
    ApplyCallback(w, call_data, client_data);
}
