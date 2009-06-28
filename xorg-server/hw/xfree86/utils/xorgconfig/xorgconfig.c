/*
 * This is a configuration program that will create a base XF86Config
 * file based on menu choices. Its main feature is that clueless users
 * may be less inclined to select crazy sync rates way over monitor spec,
 * by presenting a menu with standard monitor types. Also some people
 * don't read docs unless an executable that they can run tells them to.
 *
 * It assumes a 24-line or bigger text console.
 *
 * Revision history:
 * 25Sep94 Initial version.
 * 27Sep94 Fix hsync range of monitor types to match with best possible mode.
 *         Remove 'const'.
 *         Tweak descriptions.
 * 28Sep94 Fixes from J"org Wunsch:
 *           Don't use gets().
 *           Add mouse device prompt.
 *           Fix lines overrun for 24-line console.
 *         Increase buffer size for probeonly output.
 * 29Sep94 Fix bad bug with old XF86Config preserving during probeonly run.
 *         Add note about vertical refresh in interlaced modes.
 *         Name gets() replacement getstring().
 *         Add warning about binary paths.
 *         Fixes from David Dawes:
 *           Don't use 'ln -sf'.
 *           Omit man path reference in comment.
 *           Generate only a generic 320x200 SVGA section for accel cards.
 *	     Only allow writing to /usr/X11R6/lib/X11 if root, and use
 *	       -xf86config for the -probeonly phase (root only).
 *         Fix bug that forces screen type to accel in some cases.
 * 30Sep94 Continue after clocks probe fails.
 *         Note about programmable clocks.
 *         Rename to 'xf86config'. Not to be confused with XF86Config
 *           or the -xf86config option.
 * 07Oct94 Correct hsync in standard mode timings comments, and include
 *           the proper +/-h/vsync flags.
 * 11Oct94 Skip 'numclocks:' and 'pixel clocks:' lines when probing for
 * 	     clocks.
 * 18Oct94 Add check for existence of /usr/X11R6.
 *	   Add note about ctrl-alt-backspace.
 * 06Nov94 Add comment above standard mode timings in XF86Config.
 * 24Dec94 Add low-resolution modes using doublescan.
 * 29Dec94 Add note in horizontal sync range selection.
 *	   Ask about ClearDTR/RTS option for Mouse Systems mice.
 *	   Ask about writing to /etc/XF86Config.
 *	   Allow link to be set in /var/X11R6/bin.
 *	   Note about X -probeonly crashing.
 *	   Add keyboard Alt binding option for non-ASCII characters.
 *	   Add card database selection.
 *	   Write temporary XF86Config for clock probing in /tmp instead
 *	     of /usr/X11R6/lib/X11.
 *	   Add RAMDAC and Clockchip menu.
 * 27Mar99 Modified for XFree86 4.0 config file format
 * 06Sep02 Write comment block about 'DontVTSwitch'.
 *
 * Possible enhancements:
 * - Add more standard mode timings (also applies to README.Config). Missing
 *   are 1024x768 @ 72 Hz, 1152x900 modes, and 1280x1024 @ ~70 Hz.
 *   I suspect there is a VESA standard for 1024x768 @ 72 Hz with 77 MHz dot
 *   clock, and 1024x768 @ 75 Hz with 78.7 MHz dot clock. New types of
 *   monitors probably work better with VESA 75 Hz timings.
 * - Add option for creation of clear, minimal XF86Config.
 * - The card database doesn't include most of the entries in previous
 *   databases.
 *
 * Send comments to H.Hanemaayer@inter.nl.net.
 *
 * Things to keep up-to-date:
 * - Accelerated server names.
 * - Ramdac and Clockchip settings.
 * - The card database.
 *
 */
/*  Oct2000
 *  New 'Configuration of XKB' section.
 *  Author: Ivan Pascal      The XFree86 Project.
 */
/*
 *  Nov2002
 *  Some enhancements:
 *  - Add new PS/2 mouse protocol.
 *    "IMPS/2","ExplorerPS/2","ThinkingMousePS/2","MouseManPlusPS/2",
 *    "GlidePointPS/2","NetMousePS/2" and "NetScrollPS/2".
 *  - Add mouse-speed setting for PS/2 mouse.
 *  - Fix seg.fault problem on Solaris.
 *  - Add modestring "1400x1050"(for ATI Mobile-Rage).
 *  - Add videomemory 8192, 16384, 32768, 65536, 131072 and 262144.
 *  - Ready to DRI.
 *  - Load xtt module instead of freetype module.
 *  - Add font path "/TrueType/" and "/freefont/".
 *  Chisato Yamauchi(cyamauch@phyas.aichi-edu.ac.jp)
 */

#ifdef HAVE_CONFIG_H
# include "xorg-server.h"
# include "xkb-config.h"
#endif

#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <X11/Xlib.h>
#include <X11/extensions/XKBstr.h>
#include <X11/extensions/XKBrules.h>
#define MAX_XKBOPTIONS	5

#include "cards.h"


/*
 * Define the following to 310 to remove references to XFree86 features that
 * have been added since XFree86 3.1 (e.g. DoubleScan modes).
 * or to 311 to remove certain new modelines
 */
#define XFREE86_VERSION 400

/*
 * Define this to have /etc/X11/XF86Config prompted for as the default
 * location to write the XF86Config file to.
 */
#define PREFER_XF86CONFIG_IN_ETC

/*
 * Define this to force the user to go through XKB configuration section.
 *
 */
#define FORCE_XKB_DIALOG

/*
 * Configuration variables.
 */

#define MAX_CLOCKS_LINES 16

#define DUMBCONFIG2 "dumbconfig.2"
#define DUMBCONFIG3 "dumbconfig.3"

/* some more vars to make path names in texts more flexible. OS/2 users
 * may be more irritated than Unix users
 */
#ifndef PROJECTROOT
#define PROJECTROOT		"/usr"
#endif
#define TREEROOT		PROJECTROOT
#define TREEROOTLX		TREEROOT "/lib/X11"
#define TREEROOTCFG		TREEROOT "/etc/X11"
#define TREEROOTSHARE		TREEROOT "/share/X11"
#ifdef XDOCDIR
# define TREEROOTDOC		XDOCDIR
#else
# define TREEROOTDOC		TREEROOTLX "/doc"
#endif
#ifdef XFONTDIR
# define TREEROOTFONT		XFONTDIR
#else
# define TREEROOTFONT		TREEROOTLX "/fonts"
#endif
#define MODULEPATH		TREEROOT "/lib/modules"

#define XSERVERNAME_FOR_PROBE "X"

#ifndef XCONFIGFILE
#define XCONFIGFILE		"xorg.conf"
#endif
#define CONFIGNAME		XCONFIGFILE

/*
 * This is the filename of the temporary XF86Config file that is written
 * when the program is told to probe clocks (which can only happen for
 * root).
 */
#define TEMPORARY_XF86CONFIG_DIR_PREFIX "/tmp/."XCONFIGFILE
#define TEMPORARY_XF86CONFIG_FILENAME XCONFIGFILE".tmp"

#ifndef XF86_VERSION_MAJOR
#ifdef XVERSION
#if XVERSION > 40000000
#define XF86_VERSION_MAJOR	(XVERSION / 10000000)
#else
#define XF86_VERSION_MAJOR	(XVERSION / 1000)
#endif
#else
#define XF86_VERSION_MAJOR	4
#endif
#endif


int config_mousetype;		/* Mouse. */
int config_emulate3buttons;
int config_chordmiddle;
int config_cleardtrrts;
char *config_pointerdevice;
int config_altmeta;		/* Keyboard. */
int config_monitortype;		/* Monitor. */
char *config_hsyncrange;
char *config_vsyncrange;
char *config_monitoridentifier;
int config_videomemory;		/* Video card. */
int config_screentype;		/* mono, vga16, svga, accel */
char *config_deviceidentifier;
int config_numberofclockslines;
char *config_clocksline[MAX_CLOCKS_LINES];
char *config_modesline8bpp;
char *config_modesline16bpp;
char *config_modesline24bpp;
int config_virtual;		/* 1 (yes) or 0 (no) */
int config_virtualx8bpp, config_virtualy8bpp;
int config_virtualx16bpp, config_virtualy16bpp;
int config_virtualx24bpp, config_virtualy24bpp;
char *config_ramdac;
char *config_dacspeed;
char *config_clockchip;
#if defined(__OpenBSD__) && defined(WSCONS_SUPPORT) && !defined(PCVT_SUPPORT)
char *config_keyboard_dev = "/dev/wskbd0";
#endif
int config_xkbdisable = 0;
char *config_xkbrules;
char *config_xkbmodel = "pc105";
char *config_xkblayout = "us";
char *config_xkbvariant = (char *) 0;
char *config_xkboptions = (char *) 0;
char *config_depth;

char *temp_dir = "";

/*
 * These are from the selected card definition. Parameters from the
 * definition are offered during the questioning about the video card.
 */

int card_selected;	/* Card selected from database. */


static int write_XF86Config(char *filename);


/*
 * This is the initial intro text that appears when the program is started.
 */

static char *intro_text =
"\n"
"This program will create a basic " CONFIGNAME " file, based on menu selections\n"
"you make.  It will ask for a pathname when it is ready to write the file.\n"
"\n"
"The " CONFIGNAME " file usually resides in /etc/X11 or " TREEROOTCFG ".  If\n"
"no " CONFIGNAME " file is present there, " __XSERVERNAME__" will probe the system to\n"
"autoconfigure itself.  You can run " __XSERVERNAME__ " -configure to generate a " CONFIGNAME "\n"
"file based on the results of autoconfiguration, or let this program\n"
"produce a base " CONFIGNAME " file for your configuration, and fine-tune it.\n"
"A sample " CONFIGNAME " file is also supplied with "__XSERVERNAME__"; it is configured \n"
"for a standard VGA card and monitor with 640x480 resolution.\n"
"\n"
"There are also many chipset and card-specific options and settings available,\n"
"but this program does not know about these. On some configurations some of\n"
"these settings must be specified. Refer to the X driver man pages and the\n"
"chipset-specific READMEs in " TREEROOTDOC " for further details.\n"
#if 0
" Refer to " TREEROOTDOC "/README.Config\n"
"for a detailed overview of the configuration process.\n"
"\n"
"(what should we change this section to?)\n"
"For accelerated servers (including accelerated drivers in the SVGA server),\n"
"there are many chipset and card-specific options and settings. This program\n"
"does not know about these. On some configurations some of these settings must\n"
"be specified. Refer to the server man pages and chipset-specific READMEs.\n"
#endif
"\n"
"Before continuing with this program, make sure you know what video card\n"
"you have, and preferably also the chipset it uses and the amount of video\n"
"memory on your video card, as well as the specifications of your monitor.\n"
"\n"
;

static char *finalcomment_text =
"File has been written. Take a look at it before starting an X server. Note that\n"
"the " CONFIGNAME " file must be in one of the directories searched by the server\n"
"(e.g. /etc/X11) in order to be used. Within the server press\n"
"ctrl, alt and '+' simultaneously to cycle video resolutions. Pressing ctrl,\n"
"alt and backspace simultaneously immediately exits the server (use if\n"
"the monitor doesn't sync for a particular mode).\n"
"\n"
"For further configuration, refer to the " XCONFIGFILE "(" FILEMANSUFFIX ") manual page.\n"
"\n";

static void *
Malloc(int i) {
       void *p;

       p = malloc(i);
       if (p == NULL) {
               printf("Fatal malloc error\n");
               exit(-1);
       }
       return p;
}

static char *
Strdup(const char *s){
	char *d;

	d = Malloc(strlen(s) + 1);
	strcpy(d, s);
	return d;
}

static void 
createtmpdir(void) {
       /* length of prefix + 20 (digits in 2**64) + 1 (slash) + 1 */
       temp_dir = Malloc(strlen(TEMPORARY_XF86CONFIG_DIR_PREFIX) + 22);
       sprintf(temp_dir, "%s%ld", TEMPORARY_XF86CONFIG_DIR_PREFIX,
	       (long)getpid());
       if (mkdir(temp_dir, 0700) != 0) {
               printf("Cannot create directory %s\n", temp_dir);
               exit(-1);
       }
       /* append a slash */
       strcat(temp_dir, "/");
}


void 
keypress(void) {
	printf("Press enter to continue, or ctrl-c to abort.");
	getchar();
	printf("\n");
}

static void 
emptylines(void) {
	int i;
	for (i = 0; i < 50; i++)
		printf("\n");
}

static int 
answerisyes(char *s)
{
	if (s[0] == '\'')	/* For fools that type the ' literally. */
		return tolower(s[1]) == 'y';
	return tolower(s[0]) == 'y';
}

/*
 * This is a replacement for gets(). Limit is 80 chars.
 * The 386BSD descendants scream about using gets(), for good reason.
 */

static void 
getstring(char *s)
{
	char *cp;
	if (fgets(s, 80, stdin) == NULL)
		exit(1);
	cp = strchr(s, '\n');
	if (cp)
		*cp=0;
}

/*
 * Mouse configuration.
 */

int	M_OSMOUSE,	M_WSMOUSE,		M_AUTO,
	M_SYSMOUSE,	M_MOUSESYSTEMS, 	M_PS2,
	M_MICROSOFT,	M_BUSMOUSE,		M_IMPS2,
	M_EXPLORER_PS2, M_GLIDEPOINT_PS2,	M_MOUSEMANPLUS_PS2,
	M_NETMOUSE_PS2, M_NETSCROLL_PS2,	M_THINKINGMOUSE_PS2,
	M_ACECAD,	M_GLIDEPOINT,		M_INTELLIMOUSE,
	M_LOGITECH,	M_MMHITTAB,		M_MMSERIES,
	M_MOUSEMAN,	M_THINKINGMOUSE,	M_VUID;

struct {
	char *name;
	int *ident;
	char *desc;
} mouse_info[] = {
#if defined(QNX4)
#define DEF_PROTO_STRING	"OSMOUSE"
	{"OSMOUSE",		&M_OSMOUSE,
	 "OSMOUSE"
	},
#endif
#ifdef WSCONS_SUPPORT
#define WS_MOUSE_STRING		"wsmouse"
#define DEF_PROTO_STRING	WS_MOUSE_STRING
	{WS_MOUSE_STRING,	&M_WSMOUSE,
	 "wsmouse protocol"
	},
#endif
#ifndef DEF_PROTO_STRING
#define DEF_PROTO_STRING	"Auto"
#endif
	{"Auto",		&M_AUTO,
	 "Auto detect"
	},
#ifdef sun
	{"VUID",		&M_VUID,
	 "Solaris VUID protocol (SPARC, USB, or virtual mouse)"
	},
#endif
	{"SysMouse",		&M_SYSMOUSE,
	 "SysMouse"
	},
#define M_MOUSESYSTEMS_STRING	"MouseSystems"
	{M_MOUSESYSTEMS_STRING,	&M_MOUSESYSTEMS,
	 "Mouse Systems (3-button protocol)"
	},
	{"PS/2",		&M_PS2,
	 "PS/2 Mouse"
	},
#define M_MICROSOFT_STRING	"Microsoft"
	{M_MICROSOFT_STRING,	&M_MICROSOFT,
	 "Microsoft compatible (2-button protocol)"
	},
	{"Busmouse",		&M_BUSMOUSE,
	 "Bus Mouse"
	},
#ifndef __FreeBSD__
	{"IMPS/2",		&M_IMPS2,
	 "IntelliMouse PS/2"
	},
	{"ExplorerPS/2",	&M_EXPLORER_PS2,
	 "Explorer PS/2"
	},
	{"GlidePointPS/2",	&M_GLIDEPOINT_PS2,
	 "GlidePoint PS/2"
	},
	{"MouseManPlusPS/2",	&M_MOUSEMANPLUS_PS2,
	 "MouseManPlus PS/2"
	},
	{"NetMousePS/2",	&M_NETMOUSE_PS2,
	 "NetMouse PS/2"
	},
	{"NetScrollPS/2",	&M_NETSCROLL_PS2,
	 "NetScroll PS/2"
	},
	{"ThinkingMousePS/2",	&M_THINKINGMOUSE_PS2,
	 "ThinkingMouse PS/2"
	},
#endif
	{"AceCad",		&M_ACECAD,
	 "AceCad"
	},
	{"GlidePoint",		&M_GLIDEPOINT,
	 "GlidePoint"
	},
	{"IntelliMouse",	&M_INTELLIMOUSE,
	 "Microsoft IntelliMouse"
	},
	{"Logitech",		&M_LOGITECH,
	 "Logitech Mouse (serial, old type, Logitech protocol)"
	},
	{"MMHitTab",		&M_MMHITTAB,
	 "MM HitTablet"
	},
	{"MMSeries",		&M_MMSERIES,
	 "MM Series"	/* XXXX These descriptions should be improved. */
	},
	{"MouseMan",		&M_MOUSEMAN,
	 "Logitech MouseMan (Microsoft compatible)"
	},
	{"ThinkingMouse",	&M_THINKINGMOUSE,
	 "ThinkingMouse"
	},
};

#ifdef WSCONS_SUPPORT
# define DEF_MOUSEDEV "/dev/wsmouse";
#elif defined(__FreeBSD__) || defined(__DragonFly__)
# define DEF_MOUSEDEV "/dev/sysmouse";
#elif defined(__linux__)
# define DEF_MOUSEDEV "/dev/input/mice";
#else
# define DEF_MOUSEDEV "/dev/mouse";
#endif

static char *mouseintro_text =
"First specify a mouse protocol type. Choose one from the following list:\n"
"\n";

static char *mousedev_text =
"Now give the full device name that the mouse is connected to, for example\n"
"/dev/tty00. Just pressing enter will use the default, %s.\n"
"\n";

static char *mousecomment_text =
"The recommended protocol is " DEF_PROTO_STRING ". If you have a very old mouse\n"
"or don't want OS support or auto detection, and you have a two-button\n"
"or three-button serial mouse, it is most likely of type " M_MICROSOFT_STRING ".\n"
#ifdef WSCONS_SUPPORT
"\n"
"If your system uses the wscons console driver, with a PS/2 type mouse,\n"
"select " WS_MOUSE_STRING ".\n"
#endif
"\n";

static char *twobuttonmousecomment_text =
"You have selected a two-button mouse protocol. It is recommended that you\n"
"enable Emulate3Buttons.\n";

static char *threebuttonmousecomment_text =
"You have selected a three-button mouse protocol. It is recommended that you\n"
"do not enable Emulate3Buttons, unless the third button doesn't work.\n";

static char *unknownbuttonsmousecomment_text =
"If your mouse has only two buttons, it is recommended that you enable\n"
"Emulate3Buttons.\n";

static char *microsoftmousecomment_text =
"You have selected a Microsoft protocol mouse. If your mouse was made by\n"
"Logitech, you might want to enable ChordMiddle which could cause the\n"
"third button to work.\n";

static char *mousesystemscomment_text =
"You have selected a Mouse Systems protocol mouse. If your mouse is normally\n"
"in Microsoft-compatible mode, enabling the ClearDTR and ClearRTS options\n"
"may cause it to switch to Mouse Systems mode when the server starts.\n";

static char *logitechmousecomment_text =
"You have selected a Logitech protocol mouse. This is only valid for old\n"
"Logitech mice.\n";

static char *mousemancomment_text =
"You have selected a Logitech MouseMan type mouse. You might want to enable\n"
"ChordMiddle which could cause the third button to work.\n";

static void 
mouse_configuration(void) {

#if !defined(QNX4)
	int i, j;
	char s[80];
	char *def_mousedev = DEF_MOUSEDEV;

#define MOUSETYPE_COUNT sizeof(mouse_info)/sizeof(mouse_info[0])
	for (i = 0; i < MOUSETYPE_COUNT; i++)
		*(mouse_info[i].ident) = i;

	for (i=0;;) {
		emptylines();
		printf("%s", mouseintro_text);
		for (j = i; j < i + 14 && j < MOUSETYPE_COUNT; j++)
			printf("%2d.  %s [%s]\n", j + 1,
			       mouse_info[j].name, mouse_info[j].desc);
		printf("\n");
		printf("%s", mousecomment_text);
		printf("Enter a protocol number: ");
		getstring(s);
		if (strlen(s) == 0) {
			i += 14;
			if (i >= MOUSETYPE_COUNT)
				i = 0;
			continue;
		}
		config_mousetype = atoi(s) - 1;
		if (config_mousetype >= 0 && config_mousetype < MOUSETYPE_COUNT)
			break;
	}
	printf("\n");

	if (config_mousetype == M_LOGITECH) {
		/* Logitech. */
		printf("%s", logitechmousecomment_text);
		printf("\n");
		printf("Please answer the following question with either 'y' or 'n'.\n");
		printf("Are you sure it's really not a Microsoft compatible one? ");
		getstring(s);
		if (!answerisyes(s))
			config_mousetype = M_MICROSOFT;
		printf("\n");
	}

	config_chordmiddle = 0;
	if (config_mousetype == M_MICROSOFT || config_mousetype == M_MOUSEMAN) {
		/* Microsoft or MouseMan. */
		if (config_mousetype == M_MICROSOFT)
			printf("%s", microsoftmousecomment_text);
		else
			printf("%s", mousemancomment_text);
		printf("\n");
		printf("Please answer the following question with either 'y' or 'n'.\n");
		printf("Do you want to enable ChordMiddle? ");
		getstring(s);
		if (answerisyes(s))
			config_chordmiddle = 1;
		printf("\n");
	}

	config_cleardtrrts = 0;
	if (config_mousetype == M_MOUSESYSTEMS) {
		/* Mouse Systems. */
		printf("%s", mousesystemscomment_text);
		printf("\n");
		printf("Please answer the following question with either 'y' or 'n'.\n");
		printf("Do you want to enable ClearDTR and ClearRTS? ");
		getstring(s);
		if (answerisyes(s))
			config_cleardtrrts = 1;
		printf("\n");
	}

	if (config_mousetype == M_MICROSOFT) {
		if (config_chordmiddle)
			printf("%s", threebuttonmousecomment_text);
		else
			printf("%s", twobuttonmousecomment_text);
	}
	else if (config_mousetype == M_MOUSESYSTEMS ||
		 config_mousetype == M_INTELLIMOUSE) {
		printf("%s", threebuttonmousecomment_text);
	}
	else {
		printf("%s", unknownbuttonsmousecomment_text);
	}

	printf("\n");

	printf("Please answer the following question with either 'y' or 'n'.\n");
	printf("Do you want to enable Emulate3Buttons? ");
	getstring(s);
	if (answerisyes(s))
		config_emulate3buttons = 1;
	else
		config_emulate3buttons = 0;
	printf("\n");

#if (defined(sun) && (defined(__i386) || defined(__x86)))
	/* SPARC & USB mice (VUID or AUTO protocols) default to /dev/mouse, 
	   but PS/2 mice default to /dev/kdmouse */
	if ((config_mousetype != M_AUTO) && (config_mousetype != M_VUID)) {
	    def_mousedev = "/dev/kdmouse";
	}
#endif

	printf(mousedev_text, def_mousedev);
	printf("Mouse device: ");
	getstring(s);
	if (strlen(s) == 0) {
		config_pointerdevice = def_mousedev;
	} else {
		config_pointerdevice = Malloc(strlen(s) + 1);
		strcpy(config_pointerdevice, s);
	}
	printf("\n");

#else
       	/* set some reasonable defaults for OS/2 */
       	config_mousetype = M_OSMOUSE;
	config_chordmiddle = 0;       
	config_cleardtrrts = 0;
	config_emulate3buttons = 0;
	config_pointerdevice = "QNXMOUSE";
#endif
}


/*
 * Keyboard configuration.
 */

/*
 * Configuration of XKB 
 */
static char *xkbmodeltext = 
"Please select one of the following keyboard types that is the better\n"
"description of your keyboard. If nothing really matches,\n"
"choose \"Generic 104-key PC\"\n\n";

static char *xkblayouttext = 
"Please select the layout corresponding to your keyboard\n";

static char *xkbvarianttext =
"Please enter a variant name for '%s' layout. Or just press enter\n"
"for default variant\n\n";

static char *xkboptionstext = 
"Please answer the following question with either 'y' or 'n'.\n"
"Do you want to select additional XKB options (group switcher,\n"
"group indicator, etc.)? ";

#if defined(__OpenBSD__) && defined(WSCONS_SUPPORT) && !defined(PCVT_SUPPORT)
static char *kbdevtext =
"Please enter the device name for your keyboard or just press enter\n"
"for the default of wskbd0\n\n";
#endif

static void 
keyboard_configuration(void)
{
	int i, j;
	char s[80];
        char *rulesfile;
	int number, options[MAX_XKBOPTIONS], num_options;
        XkbRF_RulesPtr rules;

#if defined(__OpenBSD__) && defined(WSCONS_SUPPORT) && !defined(PCVT_SUPPORT)
	printf(kbdevtext);
	getstring(s);
	if (strlen(s) != 0) {
	    config_keyboard_dev = Malloc(strlen(s) + 1);
	    strcpy(config_keyboard_dev, s);
	}
#endif

#ifndef XKB_RULES_DIR
# define XKB_RULES_DIR XKB_BASE_DIRECTORY "/rules"
#endif
	
#ifdef XFREE98_XKB
	config_xkbrules = "xfree98";	/* static */
        rulesfile = XKB_RULES_DIR "/xfree98";
#else
	config_xkbrules = __XKBDEFRULES__;	/* static */
        rulesfile = XKB_RULES_DIR "/" __XKBDEFRULES__;
#endif

        rules = XkbRF_Load(rulesfile, "", True, False);
	emptylines();

        if (!rules) {
            printf("XKB rules file '%s' not found\n", rulesfile);
            printf("Keyboard XKB options will be set to default values.\n");
            keypress();
            return;
        }

	number = -1;
	for (i=0;;) {
		emptylines();
		printf(xkbmodeltext);
		for (j = i; j < i + 16 && j < rules->models.num_desc; j++)
		    printf("%3d  %-50s\n", j+1, rules->models.desc[j].desc);
		printf("\nEnter a number to choose the keyboard.\n\n");
		if (rules->models.num_desc >= 16)
			printf("Press enter for the next page\n");
		getstring(s);
		if (strlen(s) == 0) {
			i += 16;
			if (i > rules->models.num_desc)
				i = 0;
			continue;
		}
		number = atoi(s) - 1;
		if (number >= 0 && number < rules->models.num_desc)
			break;
	}

	i = strlen(rules->models.desc[number].name) + 1;
	config_xkbmodel = Malloc(i);
	sprintf(config_xkbmodel,"%s", rules->models.desc[number].name);

	emptylines();
	printf(xkblayouttext);

	number = -1;
	for (i=0;;) {
	    	emptylines();
		for (j = i; j < i + 18 && j < rules->layouts.num_desc; j++)
			printf("%3d  %-50s\n", j+1,
				rules->layouts.desc[j].desc);
		printf("\n");
		printf("Enter a number to choose the country.\n");
		if (rules->layouts.num_desc >= 18)
			printf("Press enter for the next page\n");
		printf("\n");
		getstring(s);
		if (strlen(s) == 0) {
			i += 18;
			if (i > rules->layouts.num_desc)
				i = 0;
			continue;
		}
		number = atoi(s) - 1;
		if (number >= 0 && number < rules->layouts.num_desc)
			break;
	}
	config_xkblayout = Malloc(strlen(rules->layouts.desc[number].name)+1);
	sprintf(config_xkblayout,"%s", rules->layouts.desc[number].name);

	emptylines();
	printf(xkbvarianttext, config_xkblayout);
	getstring(s);
	if (strlen(s) != 0) {
	    config_xkbvariant = Malloc(strlen(s) + 1);
	    strcpy(config_xkbvariant, s);
        }

	emptylines();
	printf(xkboptionstext);
	getstring(s);
	if (!answerisyes(s))
            return;

        num_options = 0;
        for (j=0,i=0;;) {
            if (!strchr(rules->options.desc[i].name, ':')) {
	        emptylines();
                printf("   %s\n\n", rules->options.desc[i].desc);
                j = i;
            } else {
                printf("%3d  %-50s\n", i - j, rules->options.desc[i].desc);
            }
            i++;
            if ( i == rules->options.num_desc ||
                 !strchr(rules->options.desc[i].name, ':')) {
                printf("\nPlease select the option or just press enter if none\n");
	        getstring(s);
	        if (strlen(s) != 0) {
	            number = atoi(s);
                    if (number && (num_options < MAX_XKBOPTIONS)) {
                        options[num_options++] = number + j;
                    }
                }    
            }
            if (i == rules->options.num_desc)
                break;
        }

        if (!num_options)
            return;

        for (j=0,i=0; i<num_options; i++) {
            j += strlen(rules->options.desc[options[i]].name);
        }
	config_xkboptions = Malloc(j + num_options);
        for (j=0,i=0; i<num_options; i++) {
	    j += sprintf(config_xkboptions+j,"%s%s",
                    i == 0 ? "": "," ,rules->options.desc[options[i]].name);
        }
	return;
}



/*
 * Monitor configuration.
 */

static char *monitorintro_text =
"Now we want to set the specifications of the monitor. The two critical\n"
"parameters are the vertical refresh rate, which is the rate at which the\n"
"the whole screen is refreshed, and most importantly the horizontal sync rate,\n"
"which is the rate at which scanlines are displayed.\n"
"\n"
"The valid range for horizontal sync and vertical sync should be documented\n"
"in the manual of your monitor.\n"
"\n";

static char *hsyncintro_text =
"You must indicate the horizontal sync range of your monitor. You can either\n"
"select one of the predefined ranges below that correspond to industry-\n"
"standard monitor types, or give a specific range.\n"
"\n"
"It is VERY IMPORTANT that you do not specify a monitor type with a horizontal\n"
"sync range that is beyond the capabilities of your monitor. If in doubt,\n"
"choose a conservative setting.\n"
"\n";

static char *customhsync_text =
"Please enter the horizontal sync range of your monitor, in the format used\n"
"in the table of monitor types above. You can either specify one or more\n"
"continuous ranges (e.g. 15-25, 30-50), or one or more fixed sync frequencies.\n"
"\n";

static char *vsyncintro_text =
"You must indicate the vertical sync range of your monitor. You can either\n"
"select one of the predefined ranges below that correspond to industry-\n"
"standard monitor types, or give a specific range. For interlaced modes,\n"
"the number that counts is the high one (e.g. 87 Hz rather than 43 Hz).\n"
"\n"
" 1  50-70\n"
" 2  50-90\n"
" 3  50-100\n"
" 4  40-150\n"
" 5  Enter your own vertical sync range\n";

static char *monitordescintro_text =
"You must now enter a few identification/description strings, namely an\n"
"identifier, a vendor name, and a model name. Just pressing enter will fill\n"
"in default names.\n"
"\n";

#define NU_MONITORTYPES 10

static char *monitortype_range[NU_MONITORTYPES] = {
	"31.5",
	"31.5 - 35.1",
	"31.5, 35.5",
	"31.5, 35.15, 35.5",
	"31.5 - 37.9",
	"31.5 - 48.5",
	"31.5 - 57.0",
	"31.5 - 64.3",
	"31.5 - 79.0",
	"31.5 - 82.0"
};

static char *monitortype_name[NU_MONITORTYPES] = {
	"Standard VGA, 640x480 @ 60 Hz",
	"Super VGA, 800x600 @ 56 Hz",
	"8514 Compatible, 1024x768 @ 87 Hz interlaced (no 800x600)",
	"Super VGA, 1024x768 @ 87 Hz interlaced, 800x600 @ 56 Hz",
	"Extended Super VGA, 800x600 @ 60 Hz, 640x480 @ 72 Hz",
	"Non-Interlaced SVGA, 1024x768 @ 60 Hz, 800x600 @ 72 Hz",
	"High Frequency SVGA, 1024x768 @ 70 Hz",
	"Monitor that can do 1280x1024 @ 60 Hz",
	"Monitor that can do 1280x1024 @ 74 Hz",
	"Monitor that can do 1280x1024 @ 76 Hz"
};

static void 
monitor_configuration(void) {
	int i;
	char s[80];
	printf("%s", monitorintro_text);

	keypress();
	emptylines();

	printf("%s", hsyncintro_text);

	printf("    hsync in kHz; monitor type with characteristic modes\n");
	for (i = 0; i < NU_MONITORTYPES; i++)
		printf("%2d  %s; %s\n", i + 1, monitortype_range[i],
			monitortype_name[i]);

	printf("%2d  Enter your own horizontal sync range\n",
		NU_MONITORTYPES + 1);
	printf("\n");

	printf("Enter your choice (1-%d): ", NU_MONITORTYPES + 1);
	getstring(s);
	config_monitortype = atoi(s) - 1;
	if (config_monitortype < 0)
		config_monitortype = 0;

	printf("\n");

	if (config_monitortype < NU_MONITORTYPES)
		config_hsyncrange = monitortype_range[config_monitortype];
	else {
		/* Custom hsync range option selected. */
		printf("%s", customhsync_text);
		printf("Horizontal sync range: ");
		getstring(s);
		config_hsyncrange = Malloc(strlen(s) + 1);
		strcpy(config_hsyncrange, s);
		printf("\n");
	}

	printf("%s", vsyncintro_text);
	printf("\n");

	printf("Enter your choice: ");
	getstring(s);
	printf("\n");
	switch (atoi(s)) {
	case 0 :
	case 1 :
		config_vsyncrange = "50-70";
		break;
	case 2 :
		config_vsyncrange = "50-90";
		break;
	case 3 :
		config_vsyncrange = "50-100";
		break;
	case 4 :
		config_vsyncrange = "40-150";
		break;
	case 5 :
		/* Custom vsync range option selected. */
		printf("Vertical sync range: ");
		getstring(s);
		config_vsyncrange = Malloc(strlen(s) + 1);
		strcpy(config_vsyncrange, s);
		printf("\n");
		break;
	}
	printf("%s", monitordescintro_text);
	printf("The strings are free-form, spaces are allowed.\n");
	printf("Enter an identifier for your monitor definition: ");
	getstring(s);
	if (strlen(s) == 0)
		config_monitoridentifier = "My Monitor";
	else {
		config_monitoridentifier = Malloc(strlen(s) + 1);
		strcpy(config_monitoridentifier, s);
	}
}


/*
 * Card database.
 */

static char *cardintro_text =
"Now we must configure video card specific settings. At this point you can\n"
"choose to make a selection out of a database of video card definitions.\n"
"Because there can be variation in Ramdacs and clock generators even\n"
"between cards of the same model, it is not sensible to blindly copy\n"
"the settings (e.g. a Device section). For this reason, after you make a\n"
"selection, you will still be asked about the components of the card, with\n"
"the settings from the chosen database entry presented as a strong hint.\n"
"\n"
"The database entries include information about the chipset, what driver to\n"
"run, the Ramdac and ClockChip, and comments that will be included in the\n"
"Device section. However, a lot of definitions only hint about what driver\n"
"to run (based on the chipset the card uses) and are untested.\n"
"\n"
"If you can't find your card in the database, there's nothing to worry about.\n"
"You should only choose a database entry that is exactly the same model as\n"
"your card; choosing one that looks similar is just a bad idea (e.g. a\n"
"GemStone Snail 64 may be as different from a GemStone Snail 64+ in terms of\n"
"hardware as can be).\n"
"\n";

static char *cardunsupported_text =
"This card is basically UNSUPPORTED. It may only work as a generic\n"
"VGA-compatible card. If you have an "__XSERVERNAME__" version more recent than what\n"
"this card definition was based on, there's a chance that it is now\n"
"supported.\n";

static void 
carddb_configuration(void) {
	int i;
	char s[80];
	card_selected = -1;
	printf("%s", cardintro_text);
	printf("Do you want to look at the card database? ");
	getstring(s);
	printf("\n");
	if (!answerisyes(s))
		return;

	/*
	 * Choose a database entry.
	 */
	if (parse_database()) {
		printf("Couldn't read card database file %s.\n",
			CARD_DATABASE_FILE);
		keypress();
		return;
	}

	i = 0;
	for (;;) {
		int j;
		emptylines();
		for (j = i; j < i + 18 && j <= lastcard; j++) {
			char *name = card[j].name,
			     *chipset = card[j].chipset;

			printf("%3d  %-50s%s\n", j,
				name ? name : "-",
				chipset ? chipset : "-");
		}
		printf("\n");
		printf("Enter a number to choose the corresponding card definition.\n");
		printf("Press enter for the next page, q to continue configuration.\n");
		printf("\n");
		getstring(s);
		if (s[0] == 'q')
			break;
		if (strlen(s) == 0) {
			i += 18;
			if (i > lastcard)
				i = 0;
			continue;
		}
		card_selected = atoi(s);
		if (card_selected >= 0 && card_selected <= lastcard)
			break;
	}

	/*
	 * Look at the selected card.
	 */
	if (card_selected != -1) {
		char *name = card[card_selected].name,
		     *chipset = card[card_selected].chipset;

		printf("\nYour selected card definition:\n\n");
		printf("Identifier: %s\n", name ? name : "-");
		printf("Chipset:    %s\n", chipset ? chipset : "-");
		if (!card[card_selected].driver)
			card[card_selected].driver = "unknown";
		printf("Driver:     %s\n", card[card_selected].driver);

		if (card[card_selected].ramdac != NULL)
			printf("Ramdac:     %s\n", card[card_selected].ramdac);
		if (card[card_selected].dacspeed != NULL)
			printf("DacSpeed:     %s\n", card[card_selected].dacspeed);
		if (card[card_selected].clockchip != NULL)
			printf("Clockchip:  %s\n", card[card_selected].clockchip);
		if (card[card_selected].flags & NOCLOCKPROBE)
			printf("Do NOT probe clocks or use any Clocks line.\n");
		if (card[card_selected].flags & UNSUPPORTED)
			printf("%s", cardunsupported_text);
#if 0	/* Might be confusing. */
		if (strlen(card[card_selected].lines) > 0)
			printf("Device section text:\n%s",
				card[card_selected].lines);
#endif
		printf("\n");
		keypress();
	}
}


/*
 * Screen/video card configuration.
 */

static char *deviceintro_text =
"Now you must give information about your video card. This will be used for\n"
"the \"Device\" section of your video card in " CONFIGNAME ".\n"
"\n";

static char *videomemoryintro_text =
"It is probably a good idea to use the same approximate amount as that detected\n"
"by the server you intend to use. If you encounter problems that are due to the\n"
"used server not supporting the amount memory you have, specify the maximum\n"
"amount supported by the server.\n"
"\n"
"How much video memory do you have on your video card:\n"
"\n";

static char *carddescintro_text =
"You must now enter a few identification/description strings, namely an\n"
"identifier, a vendor name, and a model name. Just pressing enter will fill\n"
"in default names (possibly from a card definition).\n"
"\n";

#if 0
static char *devicesettingscomment_text =
"Especially for accelerated drivers, Ramdac, Dacspeed and ClockChip settings\n"
"or special options may be required in the Device section.\n"
"\n";

static char *ramdaccomment_text =
"The RAMDAC setting only applies to some drivers. Some RAMDAC's are\n"
"auto-detected by the server. The detection of a RAMDAC is forced by using a\n"
"Ramdac \"identifier\" line in the Device section. The identifiers are shown\n"
"at the right of the following table of RAMDAC types:\n"
"\n";

#define NU_RAMDACS 24

static char *ramdac_name[NU_RAMDACS] = {
	"AT&T 20C490 (S3 and AGX servers, ARK driver)",
	"AT&T 20C498/21C498/22C498 (S3, autodetected)",
	"AT&T 20C409/20C499 (S3, autodetected)",
	"AT&T 20C505 (S3)",
	"BrookTree BT481 (AGX)",
	"BrookTree BT482 (AGX)",
	"BrookTree BT485/9485 (S3)",
	"Sierra SC15025 (S3, AGX)",
#if XFREE86_VERSION >= 311	
	"S3 GenDAC (86C708) (autodetected)",
	"S3 SDAC (86C716) (autodetected)",
#else
	"S3 GenDAC (86C708)",
	"S3 SDAC (86C716)",
#endif
	"STG-1700 (S3, autodetected)",
	"STG-1703 (S3, autodetected)",
	"TI 3020 (S3, autodetected)",
	"TI 3025 (S3, autodetected)",
	"TI 3026 (S3, autodetected)",
	"IBM RGB 514 (S3, autodetected)",
	"IBM RGB 524 (S3, autodetected)",
	"IBM RGB 525 (S3, autodetected)",
	"IBM RGB 526 (S3)",
	"IBM RGB 528 (S3, autodetected)",
	"ICS5342 (S3, ARK)",
	"ICS5341 (W32)",
	"IC Works w30C516 ZoomDac (ARK)",
	"Normal DAC"
};

static char *ramdac_id[NU_RAMDACS] = {
	"att20c490", "att20c498", "att20c409", "att20c505", "bt481", "bt482",
	"bt485", "sc15025", "s3gendac", "s3_sdac", "stg1700","stg1703",
	"ti3020", "ti3025", "ti3026", "ibm_rgb514", "ibm_rgb524",
	"ibm_rgb525", "ibm_rgb526", "ibm_rgb528", "ics5342", "ics5341",
	"zoomdac", "normal"
};

static char *clockchipcomment_text =
"A Clockchip line in the Device section forces the detection of a\n"
"programmable clock device. With a clockchip enabled, any required\n"
"clock can be programmed without requiring probing of clocks or a\n"
"Clocks line. Most cards don't have a programmable clock chip.\n"
"Choose from the following list:\n"
"\n";

#define NU_CLOCKCHIPS 12

static char *clockchip_name[] = {
	"Chrontel 8391",
	"ICD2061A and compatibles (ICS9161A, DCS2824)",
	"ICS2595",
	"ICS5342 (similar to SDAC, but not completely compatible)",
	"ICS5341",
	"S3 GenDAC (86C708) and ICS5300 (autodetected)",
	"S3 SDAC (86C716)",
	"STG 1703 (autodetected)",
	"Sierra SC11412",
	"TI 3025 (autodetected)",
	"TI 3026 (autodetected)",
	"IBM RGB 51x/52x (autodetected)",
};

static char *clockchip_id[] = {
	"ch8391", "icd2061a", "ics2595", "ics5342", "ics5341",
	"s3gendac", "s3_sdac",
	"stg1703", "sc11412", "ti3025", "ti3026", "ibm_rgb5xx",
};

static char *deviceclockscomment_text =
"For most modern configurations, a Clocks line is neither required or\n"
"desirable.  However for some older hardware it can be useful since it\n"
"prevents the slow and nasty sounding clock probing at server start-up.\n"
"Probed clocks are displayed at server startup, along with other server\n"
"and hardware configuration info. You can save this information in a file\n"
"by running 'X -probeonly 2>output_file'. Be warned that clock probing is\n"
"inherently imprecise; some clocks may be slightly too high (varies per run).\n"
"\n";

static char *deviceclocksquestion_text =
"At this point I can run X -probeonly, and try to extract the clock information\n"
"from the output. It is recommended that you do this yourself and if a set of\n"
"clocks is shown then you add a clocks line (note that the list of clocks may\n"
"be split over multiple Clocks lines) to your Device section afterwards. Be\n"
"aware that a clocks line is not appropriate for most modern hardware that\n"
"has programmable clocks.\n"
"\n"
"You must be root to be able to run X -probeonly now.\n"
"\n";

static char *probeonlywarning_text =
"It is possible that the hardware detection routines in the server will somehow\n"
"cause the system to crash and the screen to remain blank. If this is the\n"
"case, do not choose this option the next time. The server may need a\n"
"Ramdac, ClockChip or special option (e.g. \"nolinear\" for S3) to probe\n"
"and start-up correctly.\n"
"\n";
#endif

static char *modesorderintro_text =
"For each depth, a list of modes (resolutions) is defined. The default\n"
"resolution that the server will start-up with will be the first listed\n"
"mode that can be supported by the monitor and card.\n"
"Currently it is set to:\n"
"\n";

static char *modesorder_text2 =
"Modes that cannot be supported due to monitor or clock constraints will\n"
"be automatically skipped by the server.\n"
"\n"
" 1  Change the modes for 8-bit (256 colors)\n"
" 2  Change the modes for 16-bit (32K/64K colors)\n"
" 3  Change the modes for 24-bit (24-bit color)\n"
" 4  The modes are OK, continue.\n"
"\n";

static char *modeslist_text =
"Please type the digits corresponding to the modes that you want to select.\n"
"For example, 432 selects \"1024x768\" \"800x600\" \"640x480\", with a\n"
"default mode of 1024x768.\n"
"\n";

static char *virtual_text =
"You can have a virtual screen (desktop), which is screen area that is larger\n"
"than the physical screen and which is panned by moving the mouse to the edge\n"
"of the screen. If you don't want virtual desktop at a certain resolution,\n"
"you cannot have modes listed that are larger. Each color depth can have a\n"
"differently-sized virtual screen\n"
"\n";

static int videomemory[] = {
	256, 512, 1024, 2048, 4096, 8192, 16384, 32768, 65536, 131072, 262144
};

/* Is this required? */
#if XFREE86_VERSION >= 400
#define NU_MODESTRINGS 13
#else
#if XFREE86_VERSION >= 330
#define NU_MODESTRINGS 12
#else
#if XFREE86_VERSION >= 311
#define NU_MODESTRINGS 8
#else
#define NU_MODESTRINGS 5
#endif
#endif
#endif

static char *modestring[NU_MODESTRINGS] = {
	"\"640x400\"",
	"\"640x480\"",
	"\"800x600\"",
	"\"1024x768\"",
	"\"1280x1024\"",
#if XFREE86_VERSION >= 311
	"\"320x200\"",
	"\"320x240\"",
	"\"400x300\""
#endif
#if XFREE86_VERSION >= 330
	,"\"1152x864\"",
	"\"1600x1200\"",
	"\"1800x1400\"",
	"\"512x384\""
#endif
#if XFREE86_VERSION >= 400
	,"\"1400x1050\""
#endif
};

static int exists_dir(char *name) {
	struct stat sbuf;

	/* is it there ? */
	if (stat(name,&sbuf) == -1)
		return 0;

	/* is there, but is it a dir? */
	return S_ISDIR(sbuf.st_mode) ? 1 : 0;
}

static int 
screen_configuration(void) {
	int i, c/*, np*/;
	char s[80];

	/*
	 * Configure the "Device" section for the video card.
	 */

	printf("%s", deviceintro_text);

	printf("%s", videomemoryintro_text);

	for (i = 0; i < sizeof(videomemory) / sizeof(videomemory[0]); i++)
		printf("%2d  %dK\n", i + 1, videomemory[i]);
	printf("%2d  Other\n\n", i + 1);

	printf("Enter your choice: ");
	getstring(s);
	printf("\n");

	c = atoi(s) - 1;
	if (c >= 0 && c < sizeof(videomemory) / sizeof(videomemory[0]))
		config_videomemory = videomemory[c];
	else {
		printf("Amount of video memory in Kbytes: ");
		getstring(s);
		config_videomemory = atoi(s);
		printf("\n");
	}

	printf("%s", carddescintro_text);
	if (card_selected != -1)
		printf("Your card definition is %s.\n\n",
			card[card_selected].name);
	printf("The strings are free-form, spaces are allowed.\n");
	printf("Enter an identifier for your video card definition: ");
	getstring(s);
	if (strlen(s) == 0)
		if (card_selected != -1)
			config_deviceidentifier = card[card_selected].name;
		else
			config_deviceidentifier = "My Video Card";
	else {
		config_deviceidentifier = Malloc(strlen(s) + 1);
		strcpy(config_deviceidentifier, s);
	}
	printf("\n");

	emptylines();

	/*
	 * Initialize screen mode variables for svga and accel
	 * to default values.
	 * XXXX Doesn't leave room for off-screen caching in 16/32bpp modes
	 *      for the accelerated servers in some situations.
	 */
	config_modesline8bpp =
	config_modesline16bpp =
	config_modesline24bpp = "\"640x480\"";
	config_virtualx8bpp = config_virtualx16bpp = config_virtualx24bpp =
	config_virtualy8bpp = config_virtualy16bpp = config_virtualy24bpp = 0;
	if (config_videomemory >= 4096) {
		config_virtualx8bpp = 1600;
		config_virtualy8bpp = 1280;
		if (card_selected != -1 && !(card[card_selected].flags & UNSUPPORTED)) {
			/*
			 * Allow room for font/pixmap cache for accel
			 * servers.
			 */
			config_virtualx16bpp = 1280;
			config_virtualy16bpp = 1024;
		}
		else {
			config_virtualx16bpp = 1600;
			config_virtualy16bpp = 1280;
		}
		if (card_selected != -1 && !(card[card_selected].flags & UNSUPPORTED)) {
			config_virtualx24bpp = 1152;
			config_virtualy24bpp = 900;
		}
		else {
			config_virtualx24bpp = 1280;
			config_virtualy24bpp = 1024;
		}
		/* Add 1600x1280 */
		config_modesline8bpp = "\"1280x1024\" \"1024x768\" \"800x600\" \"640x480\"";
		config_modesline16bpp = "\"1280x1024\" \"1024x768\" \"800x600\" \"640x480\"";
		config_modesline24bpp = "\"1280x1024\" \"1024x768\" \"800x600\" \"640x480\"";

	}
	else
	if (config_videomemory >= 2048) {
		if (card_selected != -1 && !(card[card_selected].flags & UNSUPPORTED)) {
			/*
			 * Allow room for font/pixmap cache for accel
			 * servers.
			 * Also the mach32 is has a limited width.
			 */
			config_virtualx8bpp = 1280;
			config_virtualy8bpp = 1024;
		}
		else {
			config_virtualx8bpp = 1600;
			config_virtualy8bpp = 1200;
		}
		if (card_selected != -1 && !(card[card_selected].flags & UNSUPPORTED)) {
			config_virtualx16bpp = 1024;
			config_virtualy16bpp = 768;
		}
		else {
			config_virtualx16bpp = 1152;
			config_virtualy16bpp = 900;
		}
		config_virtualx24bpp = 800;
		config_virtualy24bpp = 600;
		if (config_videomemory >= 2048 + 256) {
			config_virtualx24bpp = 1024;
			config_virtualy24bpp = 768;
		}
		config_modesline8bpp = "\"1280x1024\" \"1024x768\" \"800x600\" \"640x480\"";
		config_modesline16bpp = "\"1024x768\" \"800x600\" \"640x480\"";
		if (config_videomemory >= 2048 + 256)
			config_modesline24bpp = "\"1024x768\" \"800x600\" \"640x480\"";
		else
			config_modesline24bpp = "\"800x600\" \"640x480\"";
	}
	else
	if (config_videomemory >= 1024) {
		if (card_selected != -1 && !(card[card_selected].flags & UNSUPPORTED)) {
			/*
			 * Allow room for font/pixmap cache for accel
			 * servers.
			 */
			config_virtualx8bpp = 1024;
			config_virtualy8bpp = 768;
		}
		else {
			config_virtualx8bpp = 1152;
			config_virtualy8bpp = 900;
		}
		config_virtualx16bpp = 800; /* Forget about cache space;  */
		config_virtualy16bpp = 600; /* it's small enough as it is. */
		config_virtualx24bpp = 640;
		config_virtualy24bpp = 480;
		config_modesline8bpp = "\"1024x768\" \"800x600\" \"640x480\"";
		config_modesline16bpp = "\"800x600\" \"640x480\"";
		config_modesline24bpp = "\"640x480\"";
	}
	else
	if (config_videomemory >= 512) {
		config_virtualx8bpp = 800;
		config_virtualy8bpp = 600;
		config_modesline8bpp = "\"800x600\" \"640x480\"";
		config_modesline16bpp = "\"640x400\"";
	}
	else
	if (config_videomemory >= 256) {
		config_modesline8bpp = "\"640x400\"";
		config_virtualx8bpp = 640;
		config_virtualy8bpp = 400;
	}
	else {
		printf("Invalid amount of video memory. Please try again\n");
		return(1);
	}

#if 0
	/*
	 * Handle the Ramdac/Clockchip setting.
	 */

	printf("%s", devicesettingscomment_text);

	if (card_selected == -1 || (card[card_selected].flags & UNSUPPORTED))
		goto skipramdacselection;

	printf("%s", ramdaccomment_text);

	/* meanwhile there are so many RAMDACs that they do no longer fit on
	 * on page
	 */
	for (np=12, i=0 ;;) {
		int j;
		for (j = i; j < i + np && j < NU_RAMDACS; j++)
			printf("%3d  %-60s%s\n", j+1,
				ramdac_name[j],
				ramdac_id[j]);

		printf("\n");
		if (card_selected != -1)
			if (card[card_selected].ramdac != NULL)
				printf("The card definition has Ramdac \"%s\".\n\n",
					card[card_selected].ramdac);
		printf("\n");
		printf("Enter a number to choose the corresponding RAMDAC.\n");
		printf("Press enter for the next page, q to quit without selection of a RAMDAC.\n");
		printf("\n");
		getstring(s);

		config_ramdac = NULL;
		if (s[0] == 'q')
			break;

		if (strlen(s) > 0) {
			c = atoi(s)-1;
			if (c >= 0 && c < NU_RAMDACS) {
				config_ramdac = ramdac_id[atoi(s)-1];
				break;
			}
		}
		
		i += np;
		if (np==12) np = 18; /* account intro lines only displayed 1st time */
		if (i >= NU_RAMDACS)
			i = 0;
		emptylines();
	}

skipramdacselection:
	emptylines();
	printf("%s", clockchipcomment_text);

	for (i = 0; i < NU_CLOCKCHIPS; i++)
		printf("%2d  %-60s%s\n",
			i + 1, clockchip_name[i], clockchip_id[i]);

	printf("\n");

	if (card_selected != -1)
		if (card[card_selected].clockchip != NULL)
			printf("The card definition has Clockchip \"%s\"\n\n",
				card[card_selected].clockchip);

	printf("Just press enter if you don't want a Clockchip setting.\n");
	printf("What Clockchip setting do you want (1-%d)? ", NU_CLOCKCHIPS);

	getstring(s);
	config_clockchip = NULL;
	if (strlen(s) > 0)
		config_clockchip = clockchip_id[atoi(s) - 1];

	emptylines();

	/*
	 * Optionally run X -probeonly to figure out the clocks.
	 */

	config_numberofclockslines = 0;

	printf("%s", deviceclockscomment_text);

	printf("%s", deviceclocksquestion_text);
#endif

#if 0
	/*
	 * XXX Change this to check for a CLOCKPROBE flag rather than an
	 * NOCLOCKPROBE.
	 */
	if (card_selected != -1)
		if (card[card_selected].flags & NOCLOCKPROBE)
			printf("The card definition says to NOT probe clocks.\n");

	if (config_clockchip != NULL) {
		printf("Because you have enabled a Clockchip line, there's no need for clock\n"
			"probing.\n");
		keypress();
		goto skipclockprobing;
	}

	printf("Do you want me to run 'X -probeonly' now? ");
	getstring(s);
	printf("\n");
	if (answerisyes(s)) {
		/*
		 * Write temporary XF86Config and run X -probeonly.
		 * Only allow when root.
		 */
		FILE *f;
		char *buf;
		char syscmdline[2*256+100]; /* enough */
                char *fname = NULL;
                char *d2name = NULL;
                char *d3name = NULL;

		if (getuid() != 0) {
			printf("Sorry, you must be root to do this.\n\n");
			goto endofprobeonly;
		}
		printf("%s", probeonlywarning_text);
		keypress();
		fname = Malloc(strlen(temp_dir) +
                               strlen(TEMPORARY_XF86CONFIG_FILENAME) + 1);
		sprintf(fname, "%s%s", temp_dir,
                       TEMPORARY_XF86CONFIG_FILENAME);
		d2name = Malloc(strlen(temp_dir) + strlen(DUMBCONFIG2) + 1);
		sprintf(d2name, "%s%s", temp_dir, DUMBCONFIG2);
		d3name = Malloc(strlen(temp_dir) + strlen(DUMBCONFIG3) + 1);
		sprintf(d3name, "%s%s", temp_dir, DUMBCONFIG3);
		printf("Running X -probeonly -pn -xf86config %s.\n", fname);
		write_XF86Config(fname);
		sync();
		/* compose a line with the real path */
                sprintf(syscmdline, "X -probeonly -pn -xf86config %s 2> %s",
                        fname, d2name);

		if (system(syscmdline)) {
			printf("X -probeonly call failed.\n");
			printf("No Clocks line inserted.\n");
			goto clocksprobefailed;
		}
		/* Look for 'clocks:' (case sensitive). */		
                sprintf(syscmdline, "grep clocks\\: %s > %s", d2name, d3name);
                if (system(syscmdline)) {
			printf("grep failed.\n");
			printf("Cannot find clocks in server output.\n");
			goto clocksprobefailed;
		}
                f = fopen(d3name, "r");
		buf = Malloc(8192);
		/* Parse lines. */
		while (fgets(buf, 8192, f) != NULL) {
			char *clks;
			clks = strstr(buf, "clocks: ") + 8;
			if (clks >= buf + 3 && strcmp(clks - 11, "num") == 0)
				/* Reject lines with 'numclocks:'. */
				continue;
			if (clks >= buf + 8 && strcpy(clks - 14, "pixel ") == 0)
				/* Reject lines with 'pixel clocks:'. */
				continue;
			clks[strlen(clks) - 1] = '\0';	/* Remove '\n'. */
			config_clocksline[config_numberofclockslines] =
				Malloc(strlen(clks) + 1);
			strcpy(config_clocksline[config_numberofclockslines],
				clks);
			printf("Clocks %s\n", clks);
			config_numberofclockslines++;
		}
		fclose(f);
clocksprobefailed:
                unlink(d3name);
                unlink(d2name);
                unlink(fname);
		printf("\n");

endofprobeonly:
		keypress();
	}
skipclockprobing:
#endif

	/*
	 * For vga driver, no further configuration is required.
	 */
	if (card_selected == -1 || (card[card_selected].flags & UNSUPPORTED))
		return (0);
	
	/*
	 * Configure the modes order.
	 */
	config_virtual = 0;
	for (;;) {
	 	char modes[128];

		emptylines();

		printf("%s", modesorderintro_text);
		printf("%s for 8-bit\n", config_modesline8bpp);
		printf("%s for 16-bit\n", config_modesline16bpp);
		printf("%s for 24-bit\n", config_modesline24bpp);
		printf("\n");
		printf("%s", modesorder_text2);

		printf("Enter your choice: ");
		getstring(s);
		printf("\n");

		c = atoi(s) - 1;
		if (c < 0 || c >= 3)
			break;

		printf("Select modes from the following list:\n\n");

		for (i = 0; i < NU_MODESTRINGS; i++)
                        printf(" %c  %s\n", i < 9 ? '1' + i :
                                                    'a' + i - 9, 
                                            modestring[i]);
		printf("\n");

		printf("%s", modeslist_text);

		printf("Which modes? ");
		getstring(s);
		printf("\n");

		modes[0] = '\0';
		for (i = 0; i < strlen(s); i++) {
                        if ( NU_MODESTRINGS > 9 ) {
                          if ((s[i] < '1' || s[i] > '9') &&
                              (s[i] < 'a' || s[i] > 'a' + NU_MODESTRINGS - 10)) {
                                printf("Invalid mode skipped.\n");
                                continue;
                            }
                        }
                        else {
                          if (s[i] < '1' || s[i] > '0' + NU_MODESTRINGS) {
				printf("Invalid mode skipped.\n");
				continue;
			  }
			}
			if (i > 0)
				strcat(modes, " ");
                        strcat(modes, modestring[s[i] <= '9' ? s[i] - '1' :
                                                               s[i] - 'a' + 9]);
		}
		switch (c) {
		case 0 :
			config_modesline8bpp = Malloc(strlen(modes) + 1);
			strcpy(config_modesline8bpp, modes);
			break;
		case 1 :
			config_modesline16bpp = Malloc(strlen(modes) + 1);
			strcpy(config_modesline16bpp, modes);
			break;
		case 2 :
			config_modesline24bpp = Malloc(strlen(modes) + 1);
			strcpy(config_modesline24bpp, modes);
			break;
		}

		printf("%s", virtual_text);

		printf("Please answer the following question with either 'y' or 'n'.\n");
		printf("Do you want a virtual screen that is larger than the physical screen?");
		getstring(s);
		if (answerisyes(s))
			config_virtual = 1;
	}
	return(0);
}

static char *defaultdepthtext = 
"Please specify which color depth you want to use by default:\n"
"\n";

static struct depth_str {
    char *name;
    char *desc;
} depth_list[] = {
    { "1", "1 bit (monochrome)" },
    { "4", "4 bits (16 colors)" },
    { "8", "8 bits (256 colors)" },
    { "16", "16 bits (65536 colors)" },
    { "24", "24 bits (16 million colors)" }
};

static int ndepths = sizeof(depth_list)/sizeof(struct depth_str);

static void
depth_configuration(void)
{
	int i;
	char s[80];
	int depth;

	printf(defaultdepthtext);
	for (i=0; i<ndepths; i++) {
	    printf("%3d  %-50s\n",i+1,depth_list[i].desc);
	}
	
	printf("\nEnter a number to choose the default depth.\n\n");
	getstring(s);
	if (strlen(s) == 0)
	    depth = 0;
	else {
	    i = atoi(s)-1;
	    depth = (i < 0 || i > ndepths) ? 0 : i;
	}
	config_depth = depth_list[depth].name;
}

/*
 * Create the XF86Config file.
 */

static char *XF86Config_firstchunk_text =
"# File generated by xorgconfig.\n"
"\n"
"#\n"
"# Copyright 2004 "XVENDORNAME"\n"
"#\n"
"# Permission is hereby granted, free of charge, to any person obtaining a\n"
"# copy of this software and associated documentation files (the \"Software\"),\n"
"# to deal in the Software without restriction, including without limitation\n"
"# the rights to use, copy, modify, merge, publish, distribute, sublicense,\n"
"# and/or sell copies of the Software, and to permit persons to whom the\n"
"# Software is furnished to do so, subject to the following conditions:\n"
"# \n"
"# The above copyright notice and this permission notice shall be included in\n"
"# all copies or substantial portions of the Software.\n"
"# \n"
"# THE SOFTWARE IS PROVIDED \"AS IS\", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR\n"
"# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,\n"
"# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL\n"
"# "XVENDORNAME" BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,\n"
"# WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF\n"
"# OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE\n"
"# SOFTWARE.\n"
"# \n"
"# Except as contained in this notice, the name of "XVENDORNAME" shall\n"
"# not be used in advertising or otherwise to promote the sale, use or other\n"
"# dealings in this Software without prior written authorization from\n"
"# "XVENDORNAME".\n"
"#\n"
"\n"
"# **********************************************************************\n"
"# Refer to the " CONFIGNAME "(" FILEMANSUFFIX ") man page for details about the format of \n"
"# this file.\n"
"# **********************************************************************\n"
"\n"
"# **********************************************************************\n"
"# Module section -- this  section  is used to specify\n"
"# which dynamically loadable modules to load.\n"
"# **********************************************************************\n"
"#\n"
"Section \"Module\"\n"
"\n"
"# This loads the DBE extension module.\n"
"\n"
"    Load        \"dbe\"  	# Double buffer extension\n"
"\n"
"# This loads the miscellaneous extensions module, and disables\n"
"# initialisation of the XFree86-DGA extension within that module.\n"
"    SubSection  \"extmod\"\n"
"      Option    \"omit xfree86-dga\"   # don't initialise the DGA extension\n"
"    EndSubSection\n"
"\n"
"# This loads the font modules\n"
#ifdef HAS_TYPE1
"    Load        \"type1\"\n"
#else
"#    Load        \"type1\"\n"
#endif
"    Load        \"freetype\"\n"
"#    Load        \"xtt\"\n"
"\n"
"# This loads the GLX module\n"
"#    Load       \"glx\"\n"
"# This loads the DRI module\n"
"#    Load       \"dri\"\n"
"\n"
"EndSection\n"
"\n"
"# **********************************************************************\n"
"# Files section.  This allows default font and rgb paths to be set\n"
"# **********************************************************************\n"
"\n"
"Section \"Files\"\n"
"\n"
"# Multiple FontPath entries are allowed (which are concatenated together),\n"
"# as well as specifying multiple comma-separated entries in one FontPath\n"
"# command (or a combination of both methods)\n"
"# \n"
"\n";

static char *XF86Config_fontpaths[] = 
{
/*	"    FontPath	\"" TREEROOTFONT "/75dpi/\"\n"*/
    "/local/",
	"/misc/",
	"/75dpi/:unscaled",
	"/100dpi/:unscaled",
	"/Type1/",
	"/TrueType/",
	"/freefont/",
	"/75dpi/",
	"/100dpi/",
	0 /* end of fontpaths */
};

static char *XF86Config_fontpathchunk_text =

"\n"
"# The module search path.  The default path is shown here.\n"
"\n"
"#    ModulePath \"" MODULEPATH "\"\n"
"\n"
"EndSection\n"
"\n"
"# **********************************************************************\n"
"# Server flags section.\n"
"# **********************************************************************\n"
"\n"
"Section \"ServerFlags\"\n"
"\n"
"# Uncomment this to cause a core dump at the spot where a signal is \n"
"# received.  This may leave the console in an unusable state, but may\n"
"# provide a better stack trace in the core dump to aid in debugging\n"
"\n"
"#    Option \"NoTrapSignals\"\n"
"\n"
"# Uncomment this to disable the <Ctrl><Alt><Fn> VT switch sequence\n"
"# (where n is 1 through 12).  This allows clients to receive these key\n"
"# events.\n"
"\n"
"#    Option \"DontVTSwitch\"\n"
"\n"
"# Uncomment this to disable the <Ctrl><Alt><BS> server abort sequence\n"
"# This allows clients to receive this key event.\n"
"\n"
"#    Option \"DontZap\"\n"
"\n"
"# Uncomment this to disable the <Ctrl><Alt><KP_+>/<KP_-> mode switching\n"
"# sequences.  This allows clients to receive these key events.\n"
"\n"
"#    Option \"Dont Zoom\"\n"
"\n"
"# Uncomment this to disable tuning with the xvidtune client. With\n"
"# it the client can still run and fetch card and monitor attributes,\n"
"# but it will not be allowed to change them. If it tries it will\n"
"# receive a protocol error.\n"
"\n"
"#    Option \"DisableVidModeExtension\"\n"
"\n"
"# Uncomment this to enable the use of a non-local xvidtune client. \n"
"\n"
"#    Option \"AllowNonLocalXvidtune\"\n"
"\n"
"# Uncomment this to disable dynamically modifying the input device\n"
"# (mouse and keyboard) settings. \n"
"\n"
"#    Option \"DisableModInDev\"\n"
"\n"
"# Uncomment this to enable the use of a non-local client to\n"
"# change the keyboard or mouse settings (currently only xset).\n"
"\n"
"#    Option \"AllowNonLocalModInDev\"\n"
"\n"
"EndSection\n"
"\n"
"# **********************************************************************\n"
"# Input devices\n"
"# **********************************************************************\n"
"\n"
"# **********************************************************************\n"
"# Core keyboard's InputDevice section\n"
"# **********************************************************************\n"
"\n"
"Section \"InputDevice\"\n"
"\n"
"    Identifier	\"Keyboard1\"\n"
"    Driver	\"kbd\"\n"
"\n"
"    Option \"AutoRepeat\" \"500 30\"\n"
"\n"
"# Specify which keyboard LEDs can be user-controlled (eg, with xset(1))\n"
"#    Option	\"Xleds\"      \"1 2 3\"\n"
"\n";

static char *keyboardchunk2_text =
"\n";

static char *keyboardchunk3_text =
"# To customise the XKB settings to suit your keyboard, modify the\n"
"# lines below (which are the defaults).  For example, for a non-U.S.\n"
"# keyboard, you will probably want to use:\n"
"#    Option \"XkbModel\"    \"pc105\"\n"
"# If you have a US Microsoft Natural keyboard, you can use:\n"
"#    Option \"XkbModel\"    \"microsoft\"\n"
"#\n"
"# Then to change the language, change the Layout setting.\n"
"# For example, a german layout can be obtained with:\n"
"#    Option \"XkbLayout\"   \"de\"\n"
"# or:\n"
"#    Option \"XkbLayout\"   \"de\"\n"
"#    Option \"XkbVariant\"  \"nodeadkeys\"\n"
"#\n"
"# If you'd like to switch the positions of your capslock and\n"
"# control keys, use:\n"
"#    Option \"XkbOptions\"  \"ctrl:swapcaps\"\n"
"\n"
"# These are the default XKB settings for "__XSERVERNAME__"\n"
"#    Option \"XkbRules\"    \""__XKBDEFRULES__"\"\n"
"#    Option \"XkbModel\"    \"pc105\"\n"
"#    Option \"XkbLayout\"   \"us\"\n"
"#    Option \"XkbVariant\"  \"\"\n"
"#    Option \"XkbOptions\"  \"\"\n"
"\n";

static char *keyboardlastchunk_text =
"\n"
"EndSection\n"
"\n"
"\n";

static char *pointersection_text1 = 
"# **********************************************************************\n"
"# Core Pointer's InputDevice section\n"
"# **********************************************************************\n"
"\n"
"Section \"InputDevice\"\n"
"\n"
"# Identifier and driver\n"
"\n"
#if defined(__UNIXWARE__)
"#    Identifier	\"Mouse1\"\n"
"#    Driver	\"mouse\"\n"
#else
"    Identifier	\"Mouse1\"\n"
"    Driver	\"mouse\"\n"
#endif
;

static char *pointersection_text2 =
"\n"
"# Mouse-speed setting for PS/2 mouse.\n"
"\n"
"#    Option \"Resolution\"	\"256\"\n"
"\n"
"# Baudrate and SampleRate are only for some Logitech mice. In\n"
"# almost every case these lines should be omitted.\n"
"\n"
"#    Option \"BaudRate\"	\"9600\"\n"
"#    Option \"SampleRate\"	\"150\"\n"
"\n"
"# Mouse wheel mapping.  Default is to map vertical wheel to buttons 4 & 5,\n"
"# horizontal wheel to buttons 6 & 7.   Change if your mouse has more than\n"
"# 3 buttons and you need to map the wheel to different button ids to avoid\n"
"# conflicts.\n"
"\n"
"    Option \"ZAxisMapping\"   \"4 5 6 7\"\n"
"\n"
"# Emulate3Buttons is an option for 2-button mice\n"
"# Emulate3Timeout is the timeout in milliseconds (default is 50ms)\n"
"\n";


static char *xinputsection_text =
"# **********************************************************************\n"
"# Other input device sections \n"
"# this is optional and is required only if you\n"
"# are using extended input devices.  This is for example only.  Refer\n"
"# to the " CONFIGNAME " man page for a description of the options.\n"
"# **********************************************************************\n"
"#\n"
"# Section \"InputDevice\" \n"
"#    Identifier  \"Mouse2\"\n"
"#    Driver      \"mouse\"\n"
"#    Option      \"Protocol\"      \"MouseMan\"\n"
"#    Option      \"Device\"        \"/dev/mouse2\"\n"
"# EndSection\n"
"#\n"
"# Section \"InputDevice\"\n"
"#    Identifier \"spaceball\"\n"
"#    Driver     \"magellan\"\n"
"#    Option     \"Device\"        \"/dev/cua0\"\n"
"# EndSection\n"
"#\n"
"# Section \"InputDevice\"\n"
"#    Identifier \"spaceball2\"\n"
"#    Driver     \"spaceorb\"\n"
"#    Option     \"Device\"        \"/dev/cua0\"\n"
"# EndSection\n"
"#\n"
"# Section \"InputDevice\"\n"
"#    Identifier \"touchscreen0\"\n"
"#    Driver     \"microtouch\"\n"
"#    Option     \"Device\"        \"/dev/ttyS0\"\n"
"#    Option     \"MinX\"          \"1412\"\n"
"#    Option     \"MaxX\"          \"15184\"\n"
"#    Option     \"MinY\"          \"15372\"\n"
"#    Option     \"MaxY\"          \"1230\"\n"
"#    Option     \"ScreenNumber\"  \"0\"\n"
"#    Option     \"ReportingMode\" \"Scaled\"\n"
"#    Option     \"ButtonNumber\"  \"1\"\n"
"#    Option     \"SendCoreEvents\"\n"
"# EndSection\n"
"#\n"
"# Section \"InputDevice\"\n"
"#    Identifier \"touchscreen1\"\n"
"#    Driver     \"elo2300\"\n"
"#    Option     \"Device\"        \"/dev/ttyS0\"\n"
"#    Option     \"MinX\"          \"231\"\n"
"#    Option     \"MaxX\"          \"3868\"\n"
"#    Option     \"MinY\"          \"3858\"\n"
"#    Option     \"MaxY\"          \"272\"\n"
"#    Option     \"ScreenNumber\"  \"0\"\n"
"#    Option     \"ReportingMode\" \"Scaled\"\n"
"#    Option     \"ButtonThreshold\"       \"17\"\n"
"#    Option     \"ButtonNumber\"  \"1\"\n"
"#    Option     \"SendCoreEvents\"\n"
"# EndSection\n"
"\n";

static char *monitorsection_text1 =
"# **********************************************************************\n"
"# Monitor section\n"
"# **********************************************************************\n"
"\n"
"# Any number of monitor sections may be present\n"
"\n"
"Section \"Monitor\"\n"
"\n";

static char *monitorsection_text2 =
"# HorizSync is in kHz unless units are specified.\n"
"# HorizSync may be a comma separated list of discrete values, or a\n"
"# comma separated list of ranges of values.\n"
"# NOTE: THE VALUES HERE ARE EXAMPLES ONLY.  REFER TO YOUR MONITOR\'S\n"
"# USER MANUAL FOR THE CORRECT NUMBERS.\n"
"\n";

static char *monitorsection_text3 =
"#    HorizSync	30-64         # multisync\n"
"#    HorizSync	31.5, 35.2    # multiple fixed sync frequencies\n"
"#    HorizSync	15-25, 30-50  # multiple ranges of sync frequencies\n"
"\n"
"# VertRefresh is in Hz unless units are specified.\n"
"# VertRefresh may be a comma separated list of discrete values, or a\n"
"# comma separated list of ranges of values.\n"
"# NOTE: THE VALUES HERE ARE EXAMPLES ONLY.  REFER TO YOUR MONITOR\'S\n"
"# USER MANUAL FOR THE CORRECT NUMBERS.\n"
"\n";

#if 0
static char *monitorsection_text4 =
"# Modes can be specified in two formats.  A compact one-line format, or\n"
"# a multi-line format.\n"
"\n"
"# These two are equivalent\n"
"\n"
"#    ModeLine \"1024x768i\" 45 1024 1048 1208 1264 768 776 784 817 Interlace\n"
"\n"
"#    Mode \"1024x768i\"\n"
"#        DotClock	45\n"
"#        HTimings	1024 1048 1208 1264\n"
"#        VTimings	768 776 784 817\n"
"#        Flags		\"Interlace\"\n"
"#    EndMode\n"
"\n";

static char *modelines_text =
"# This is a set of standard mode timings. Modes that are out of monitor spec\n"
"# are automatically deleted by the server (provided the HorizSync and\n"
"# VertRefresh lines are correct), so there's no immediate need to\n"
"# delete mode timings (unless particular mode timings don't work on your\n"
"# monitor). With these modes, the best standard mode that your monitor\n"
"# and video card can support for a given resolution is automatically\n"
"# used.\n"
"\n"
"# 640x400 @ 70 Hz, 31.5 kHz hsync\n"
"Modeline \"640x400\"     25.175 640  664  760  800   400  409  411  450\n"
"# 640x480 @ 60 Hz, 31.5 kHz hsync\n"
"Modeline \"640x480\"     25.175 640  664  760  800   480  491  493  525\n"
"# 800x600 @ 56 Hz, 35.15 kHz hsync\n"
"ModeLine \"800x600\"     36     800  824  896 1024   600  601  603  625\n"
"# 1024x768 @ 87 Hz interlaced, 35.5 kHz hsync\n"
"Modeline \"1024x768\"    44.9  1024 1048 1208 1264   768  776  784  817 Interlace\n"
"\n"
"# 640x400 @ 85 Hz, 37.86 kHz hsync\n"
"Modeline \"640x400\"     31.5   640  672 736   832   400  401  404  445 -HSync +VSync\n"
"# 640x480 @ 72 Hz, 36.5 kHz hsync\n"
"Modeline \"640x480\"     31.5   640  680  720  864   480  488  491  521\n"
"# 640x480 @ 75 Hz, 37.50 kHz hsync\n"
"ModeLine  \"640x480\"    31.5   640  656  720  840   480  481  484  500 -HSync -VSync\n"
"# 800x600 @ 60 Hz, 37.8 kHz hsync\n"
"Modeline \"800x600\"     40     800  840  968 1056   600  601  605  628 +hsync +vsync\n"
"\n"
"# 640x480 @ 85 Hz, 43.27 kHz hsync\n"
"Modeline \"640x480\"     36     640  696  752  832   480  481  484  509 -HSync -VSync\n"
"# 1152x864 @ 89 Hz interlaced, 44 kHz hsync\n"
"ModeLine \"1152x864\"    65    1152 1168 1384 1480   864  865  875  985 Interlace\n"
"\n"
"# 800x600 @ 72 Hz, 48.0 kHz hsync\n"
"Modeline \"800x600\"     50     800  856  976 1040   600  637  643  666 +hsync +vsync\n"
"# 1024x768 @ 60 Hz, 48.4 kHz hsync\n"
"Modeline \"1024x768\"    65    1024 1032 1176 1344   768  771  777  806 -hsync -vsync\n"
"\n"
"# 640x480 @ 100 Hz, 53.01 kHz hsync\n"
"Modeline \"640x480\"     45.8   640  672  768  864   480  488  494  530 -HSync -VSync\n"
"# 1152x864 @ 60 Hz, 53.5 kHz hsync\n"
"Modeline  \"1152x864\"   89.9  1152 1216 1472 1680   864  868  876  892 -HSync -VSync\n"
"# 800x600 @ 85 Hz, 55.84 kHz hsync\n"
"Modeline  \"800x600\"    60.75  800  864  928 1088   600  616  621  657 -HSync -VSync\n"
"\n"
"# 1024x768 @ 70 Hz, 56.5 kHz hsync\n"
"Modeline \"1024x768\"    75    1024 1048 1184 1328   768  771  777  806 -hsync -vsync\n"
"# 1280x1024 @ 87 Hz interlaced, 51 kHz hsync\n"
"Modeline \"1280x1024\"   80    1280 1296 1512 1568  1024 1025 1037 1165 Interlace\n"
"\n"
"# 800x600 @ 100 Hz, 64.02 kHz hsync\n"
"Modeline  \"800x600\"    69.65  800  864  928 1088   600  604  610  640 -HSync -VSync\n"
"# 1024x768 @ 76 Hz, 62.5 kHz hsync\n"
"Modeline \"1024x768\"    85    1024 1032 1152 1360   768  784  787  823\n"
"# 1152x864 @ 70 Hz, 62.4 kHz hsync\n"
"Modeline  \"1152x864\"   92    1152 1208 1368 1474   864  865  875  895\n"
"# 1280x1024 @ 61 Hz, 64.2 kHz hsync\n"
"Modeline \"1280x1024\"  110    1280 1328 1512 1712  1024 1025 1028 1054\n"
"\n"
"# 1024x768 @ 85 Hz, 70.24 kHz hsync\n"
"Modeline \"1024x768\"   98.9  1024 1056 1216 1408   768 782 788 822 -HSync -VSync\n"
"# 1152x864 @ 78 Hz, 70.8 kHz hsync\n"
"Modeline \"1152x864\"   110   1152 1240 1324 1552   864  864  876  908\n"
"\n"
"# 1280x1024 @ 70 Hz, 74.59 kHz hsync\n"
"Modeline \"1280x1024\"  126.5 1280 1312 1472 1696  1024 1032 1040 1068 -HSync -VSync\n"
"# 1600x1200 @ 60Hz, 75.00 kHz hsync\n"
"Modeline \"1600x1200\"  162   1600 1664 1856 2160  1200 1201 1204 1250 +HSync +VSync\n"
"# 1152x864 @ 84 Hz, 76.0 kHz hsync\n"
"Modeline \"1152x864\"   135    1152 1464 1592 1776   864  864  876  908\n"
"\n"
"# 1280x1024 @ 74 Hz, 78.85 kHz hsync\n"
"Modeline \"1280x1024\"  135    1280 1312 1456 1712  1024 1027 1030 1064\n"
"\n"
"# 1024x768 @ 100Hz, 80.21 kHz hsync\n"
"Modeline \"1024x768\"   115.5  1024 1056 1248 1440  768  771  781  802 -HSync -VSync\n"
"# 1280x1024 @ 76 Hz, 81.13 kHz hsync\n"
"Modeline \"1280x1024\"  135    1280 1312 1416 1664  1024 1027 1030 1064\n"
"\n"
"# 1600x1200 @ 70 Hz, 87.50 kHz hsync\n"
"Modeline \"1600x1200\"  189    1600 1664 1856 2160  1200 1201 1204 1250 -HSync -VSync\n"
"# 1152x864 @ 100 Hz, 89.62 kHz hsync\n"
"Modeline \"1152x864\"   137.65 1152 1184 1312 1536   864  866  885  902 -HSync -VSync\n"
"# 1280x1024 @ 85 Hz, 91.15 kHz hsync\n"
"Modeline \"1280x1024\"  157.5  1280 1344 1504 1728  1024 1025 1028 1072 +HSync +VSync\n"
"# 1600x1200 @ 75 Hz, 93.75 kHz hsync\n"
"Modeline \"1600x1200\"  202.5  1600 1664 1856 2160  1200 1201 1204 1250 +HSync +VSync\n"
"# 1600x1200 @ 85 Hz, 105.77 kHz hsync\n"
"Modeline \"1600x1200\"  220    1600 1616 1808 2080  1200 1204 1207 1244 +HSync +VSync\n"
"# 1280x1024 @ 100 Hz, 107.16 kHz hsync\n"
"Modeline \"1280x1024\"  181.75 1280 1312 1440 1696  1024 1031 1046 1072 -HSync -VSync\n"
"\n"
"# 1800x1440 @ 64Hz, 96.15 kHz hsync \n"
"ModeLine \"1800X1440\"  230    1800 1896 2088 2392 1440 1441 1444 1490 +HSync +VSync\n"
"# 1800x1440 @ 70Hz, 104.52 kHz hsync \n"
"ModeLine \"1800X1440\"  250    1800 1896 2088 2392 1440 1441 1444 1490 +HSync +VSync\n"
"\n"
"# 512x384 @ 78 Hz, 31.50 kHz hsync\n"
"Modeline \"512x384\"    20.160 512  528  592  640   384  385  388  404 -HSync -VSync\n"
"# 512x384 @ 85 Hz, 34.38 kHz hsync\n"
"Modeline \"512x384\"    22     512  528  592  640   384  385  388  404 -HSync -VSync\n"
"\n"
#if XFREE86_VERSION >= 311
"# Low-res Doublescan modes\n"
"# If your chipset does not support doublescan, you get a 'squashed'\n"
"# resolution like 320x400.\n"
"\n"
"# 320x200 @ 70 Hz, 31.5 kHz hsync, 8:5 aspect ratio\n"
"Modeline \"320x200\"     12.588 320  336  384  400   200  204  205  225 Doublescan\n"
"# 320x240 @ 60 Hz, 31.5 kHz hsync, 4:3 aspect ratio\n"
"Modeline \"320x240\"     12.588 320  336  384  400   240  245  246  262 Doublescan\n"
"# 320x240 @ 72 Hz, 36.5 kHz hsync\n"
"Modeline \"320x240\"     15.750 320  336  384  400   240  244  246  262 Doublescan\n"
"# 400x300 @ 56 Hz, 35.2 kHz hsync, 4:3 aspect ratio\n"
"ModeLine \"400x300\"     18     400  416  448  512   300  301  302  312 Doublescan\n"
"# 400x300 @ 60 Hz, 37.8 kHz hsync\n"
"Modeline \"400x300\"     20     400  416  480  528   300  301  303  314 Doublescan\n"
"# 400x300 @ 72 Hz, 48.0 kHz hsync\n"
"Modeline \"400x300\"     25     400  424  488  520   300  319  322  333 Doublescan\n"
"# 480x300 @ 56 Hz, 35.2 kHz hsync, 8:5 aspect ratio\n"
"ModeLine \"480x300\"     21.656 480  496  536  616   300  301  302  312 Doublescan\n"
"# 480x300 @ 60 Hz, 37.8 kHz hsync\n"
"Modeline \"480x300\"     23.890 480  496  576  632   300  301  303  314 Doublescan\n"
"# 480x300 @ 63 Hz, 39.6 kHz hsync\n"
"Modeline \"480x300\"     25     480  496  576  632   300  301  303  314 Doublescan\n"
"# 480x300 @ 72 Hz, 48.0 kHz hsync\n"
"Modeline \"480x300\"     29.952 480  504  584  624   300  319  322  333 Doublescan\n"
"\n"
#endif
;
#endif

static char *devicesection_text =
"# **********************************************************************\n"
"# Graphics device section\n"
"# **********************************************************************\n"
"\n"
"# Any number of graphics device sections may be present\n"
"\n"
"# Standard VGA Device:\n"
"\n"
"Section \"Device\"\n"
"    Identifier	\"Standard VGA\"\n"
"    VendorName	\"Unknown\"\n"
"    BoardName	\"Unknown\"\n"
"\n"
"# The chipset line is optional in most cases.  It can be used to override\n"
"# the driver's chipset detection, and should not normally be specified.\n"
"\n"
"#    Chipset	\"generic\"\n"
"\n"
"# The Driver line must be present.  When using run-time loadable driver\n"
"# modules, this line instructs the server to load the specified driver\n"
"# module.  Even when not using loadable driver modules, this line\n"
"# indicates which driver should interpret the information in this section.\n"
"\n"
"    Driver     \"vga\"\n"
"# The BusID line is used to specify which of possibly multiple devices\n"
"# this section is intended for.  When this line isn't present, a device\n"
"# section can only match up with the primary video device.  For PCI\n"
"# devices a line like the following could be used.  This line should not\n"
"# normally be included unless there is more than one video device\n"
"# intalled.\n"
"\n"
"#    BusID      \"PCI:0:10:0\"\n"
"\n"
"#    VideoRam	256\n"
"\n"
"#    Clocks	25.2 28.3\n"
"\n"
"EndSection\n"
"\n"
"# Device configured by xorgconfig:\n"
"\n";

static char *screensection_text1 =
"# **********************************************************************\n"
"# Screen sections\n"
"# **********************************************************************\n"
"\n"
"# Any number of screen sections may be present.  Each describes\n"
"# the configuration of a single screen.  A single specific screen section\n"
"# may be specified from the X server command line with the \"-screen\"\n"
"# option.\n";

static char *serverlayout_section_text1 = 
"# **********************************************************************\n"
"# ServerLayout sections.\n"
"# **********************************************************************\n"
"\n"
"# Any number of ServerLayout sections may be present.  Each describes\n"
"# the way multiple screens are organised.  A specific ServerLayout\n"
"# section may be specified from the X server command line with the\n"
"# \"-layout\" option.  In the absence of this, the first section is used.\n"
"# When now ServerLayout section is present, the first Screen section\n"
"# is used alone.\n"
"\n"
"Section \"ServerLayout\"\n"
"\n"
"# The Identifier line must be present\n"
"    Identifier  \"Simple Layout\"\n"
"\n"
"# Each Screen line specifies a Screen section name, and optionally\n"
"# the relative position of other screens.  The four names after\n"
"# primary screen name are the screens to the top, bottom, left and right\n"
"# of the primary screen.  In this example, screen 2 is located to the\n"
"# right of screen 1.\n"
"\n";

static char *serverlayout_section_text2 =
"\n"
"# Each InputDevice line specifies an InputDevice section name and\n"
"# optionally some options to specify the way the device is to be\n"
"# used.  Those options include \"CorePointer\", \"CoreKeyboard\" and\n"
"# \"SendCoreEvents\".\n"
"\n"
"    InputDevice \"Mouse1\" \"CorePointer\"\n"
"    InputDevice \"Keyboard1\" \"CoreKeyboard\"\n"
"\n"
"EndSection\n"
"\n"
"# Section \"DRI\"\n"
"#    Mode 0666\n"
"# EndSection\n"
"\n";

static void 
write_fontpath_section(FILE *f)
{
	/* this will create the Fontpath lines, but only after checking,
	 * that the corresponding dir exists (was THE absolute problem
	 * users had with XFree86/OS2 3.1.2D !)
	 */
	int i;
	char cur[256+20],*colon, *hash;

#ifdef COMPILEDDEFAULTFONTPATH
	static const char dfp[] = COMPILEDDEFAULTFONTPATH;
	const char *thisdir;
	const char *nextdir;
	int len;

	for (thisdir = dfp; thisdir != NULL; thisdir = nextdir) {
	    nextdir = strchr(thisdir, ',');
	    if (nextdir == NULL) {
		len = strlen(thisdir);
	    } else {
		len = nextdir - thisdir;
		nextdir++;
	    }
	    if (len >= sizeof(cur))
		continue;
	    strncpy(cur, thisdir, len);
	    cur[len] = '\0';
	    colon = strchr(cur+2,':'); /* OS/2: C:/...:scaled */
	    if (colon) *colon = 0;
	    hash = exists_dir(cur) ? "" : "#";
	    if (colon) *colon = ':';
	    fprintf(f,"%s    FontPath   \"%s\"\n", hash, cur);
	}
#endif

	for (i=0; XF86Config_fontpaths[i]; i++) {
		strcpy(cur,TREEROOTFONT);
		strcat(cur,XF86Config_fontpaths[i]);
		/* remove a ':' */
		colon = strchr(cur+2,':'); /* OS/2: C:/...:scaled */
		if (colon) *colon = 0;
#ifdef COMPILEDDEFAULTFONTPATH
		/* skip if we already added it as part of the default font path */
		if (strstr(dfp, cur) != NULL)
		    continue;
#endif
		hash = exists_dir(cur) ? "" : "#";
		fprintf(f,"%s    FontPath   \"%s%s\"\n",
			hash,
			TREEROOTFONT,
			XF86Config_fontpaths[i]);
	}
}

static int 
write_XF86Config(char *filename)
{
	FILE *f;

	/*
	 * Write the file.
	 */

	f = fopen(filename, "w");
	if (f == NULL) {
		printf("Failed to open filename for writing.\n");
		if (getuid() != 0)
			printf("Maybe you need to be root to write to the specified directory?\n");
		return(1);
	}

	fprintf(f, "%s", XF86Config_firstchunk_text);
	write_fontpath_section(f);
	fprintf(f, "%s", XF86Config_fontpathchunk_text);

	/*
	 * Write keyboard section.
	 */
	if (config_altmeta) {
		fprintf(f, "    Option \"LeftAlt\"     \"Meta\"\n");
		fprintf(f, "    Option \"RightAlt\"    \"ModeShift\"\n");
	}
	else {
		fprintf(f, "#    Option \"LeftAlt\"     \"Meta\"\n");
		fprintf(f, "#    Option \"RightAlt\"    \"ModeShift\"\n");
	}
#if defined(__OpenBSD__) && defined(WSCONS_SUPPORT) && !defined(PCVT_SUPPORT)
	/* wscons keyoards need a protocol line */
	fprintf(f, "    Option \"Protocol\" \"wskbd\"\n");
	fprintf(f, "    Option \"Device\" \"%s\"\n", config_keyboard_dev);
	fprintf(f, "    Option \"XkbKeycodes\" \"wscons(ppc)\"\n");
#endif
	fprintf(f, "%s", keyboardchunk2_text);

	fprintf(f, "%s", keyboardchunk3_text);
	if (config_xkbdisable) {
		fprintf(f, "    Option \"XkbDisable\"\n\n");
	} else {
		fprintf(f, "#    Option \"XkbDisable\"\n\n");
	}
	fprintf(f, "    Option \"XkbRules\"	\"%s\"\n",
		config_xkbrules);
	fprintf(f, "    Option \"XkbModel\"	\"%s\"\n",
		config_xkbmodel);
	fprintf(f, "    Option \"XkbLayout\"	\"%s\"\n",
		config_xkblayout);
        if (config_xkbvariant)
	    fprintf(f, "    Option \"XkbVariant\"	\"%s\"\n",
		    config_xkbvariant);
        if (config_xkboptions)
	    fprintf(f, "    Option \"XkbOptions\"	\"%s\"\n",
		    config_xkboptions);

	fprintf(f, "%s",keyboardlastchunk_text);

	/*
	 * Write pointer section.
	 */
	fprintf(f, "%s", pointersection_text1);
	fprintf(f, "    Option \"Protocol\"    \"%s\"\t# %s\n",
		mouse_info[config_mousetype].name,
		mouse_info[config_mousetype].desc);
#if !defined(QNX4)
	fprintf(f, "    Option \"Device\"      \"%s\"\n", config_pointerdevice);
#endif
	fprintf(f, "%s", pointersection_text2);
	if (!config_emulate3buttons)
		fprintf(f, "#");
	fprintf(f, "    Option \"Emulate3Buttons\"\n");
	fprintf(f, "#    Option \"Emulate3Timeout\"    \"50\"\n\n");
	fprintf(f, "# ChordMiddle is an option for some 3-button Logitech mice\n\n");
	if (!config_chordmiddle)
		fprintf(f, "#");
	fprintf(f, "    Option \"ChordMiddle\"\n\n");
	if (config_cleardtrrts) {
		fprintf(f, "    Option \"ClearDTR\"\n");
		fprintf(f, "    Option \"ClearRTS\"\n\n");
	}
	fprintf(f, "EndSection\n\n\n");

	/*
	 * Write XInput sample section
	 */
	fprintf(f, "%s", xinputsection_text);

	/*
	 * Write monitor section.
	 */
	fprintf(f, "%s", monitorsection_text1);
	fprintf(f, "    Identifier  \"%s\"\n", config_monitoridentifier);
	fprintf(f, "\n");
	fprintf(f, "%s", monitorsection_text2);
	fprintf(f, "    HorizSync   %s\n", config_hsyncrange);
	fprintf(f, "\n");
	fprintf(f, "%s", monitorsection_text3);
	fprintf(f, "    VertRefresh %s\n", config_vsyncrange);
	fprintf(f, "\n");
#if 0
	fprintf(f, "%s", monitorsection_text4);
	fprintf(f, "%s", modelines_text);
#endif
	fprintf(f, "EndSection\n\n\n");

	/*
	 * Write Device section.
	 */

	fprintf(f, "%s", devicesection_text);
	fprintf(f, "Section \"Device\"\n");
	fprintf(f, "    Identifier  \"%s\"\n", config_deviceidentifier);
   	if (card_selected != -1) {
	        fprintf(f, "    Driver      \"%s\"\n", card[card_selected].driver);
	        if (card[card_selected].flags & UNSUPPORTED) {
			fprintf(f, "	# unsupported card\n");
		}
	} else {
	        fprintf(f, "    Driver      \"vga\"\n"
			"	# unsupported card\n");
	}
	/* Rely on server to detect video memory. */
	fprintf(f, "    #VideoRam    %d\n", config_videomemory);
	if (card_selected != -1)
		/* Add comment lines from card definition. */
		fprintf(f, card[card_selected].lines);
	if (config_ramdac != NULL)
		fprintf(f, "    Ramdac      \"%s\"\n", config_ramdac);
	if (card_selected != -1)
		if (card[card_selected].dacspeed != NULL)
			fprintf(f, "    Dacspeed    %s\n",
				card[card_selected].dacspeed);
	if (config_clockchip != NULL)
		fprintf(f, "    Clockchip   \"%s\"\n", config_clockchip);
	else
	if (config_numberofclockslines == 0)
		fprintf(f, "    # Insert Clocks lines here if appropriate\n");
	else {
		int i;
		for (i = 0; i < config_numberofclockslines; i++)
			fprintf(f, "    Clocks %s\n", config_clocksline[i]);
	}
	fprintf(f, "EndSection\n\n\n");	

	/*
	 * Write Screen sections.
	 */

	fprintf(f, "%s", screensection_text1);

	fprintf(f, 
		"Section \"Screen\"\n"
		"    Identifier  \"Screen 1\"\n"
		"    Device      \"%s\"\n"
		"    Monitor     \"%s\"\n"
		"    DefaultDepth %s\n"
		"\n"
		"    Subsection \"Display\"\n"
		"        Depth       8\n"
		"        Modes       %s\n"
		"        ViewPort    0 0\n",
		config_deviceidentifier,
		config_monitoridentifier,
		config_depth,
		config_modesline8bpp);
	if (config_virtual)
		fprintf(f, "        Virtual     %d %d\n",
			config_virtualx8bpp, config_virtualy8bpp);
	fprintf(f, 
		"    EndSubsection\n"
		"    Subsection \"Display\"\n"
		"        Depth       16\n"
		"        Modes       %s\n"
		"        ViewPort    0 0\n",
		config_modesline16bpp);
	if (config_virtual)
		fprintf(f, "        Virtual     %d %d\n",
			config_virtualx16bpp, config_virtualy16bpp);
	fprintf(f, 
		"    EndSubsection\n"
		"    Subsection \"Display\"\n"
		"        Depth       24\n"
		"        Modes       %s\n"
		"        ViewPort    0 0\n",
		config_modesline24bpp);
	if (config_virtual)
		fprintf(f, "        Virtual     %d %d\n",
			config_virtualx24bpp, config_virtualy24bpp);
	fprintf(f, 
		"    EndSubsection\n"
		"EndSection\n"
		"\n");

        /*
	 * ServerLayout section
	 */
   
        fprintf(f, serverlayout_section_text1);
	/* replace with  screen config */
	fprintf(f, "    Screen \"Screen 1\"\n");

	fprintf(f, serverlayout_section_text2);

        fclose(f);
	return(0);
}

static char *
append_version(char *name)
{
#ifdef APPEND_VERSION_TO_CONFIG_NAME
	char *ret = NULL;

	if (XF86_VERSION_MAJOR > 9 || XF86_VERSION_MAJOR < 0)
		return name;

	ret = Malloc(strlen(name) + 2 + 1);
	sprintf(ret, "%s-%d", name, XF86_VERSION_MAJOR);
	free(name);
	return ret;
#else
	return name;
#endif
}

/*
 * Ask where to write XF86Config to. Returns filename.
 */

static char *
ask_XF86Config_location(void) {
	char s[80];
	char *filename = NULL;

	printf(
"I am going to write the " CONFIGNAME " file now. Make sure you don't accidently\n"
"overwrite a previously configured one.\n\n");

	if (getuid() == 0) {
#ifdef PREFER_XF86CONFIG_IN_ETC
		filename = Strdup("/etc/X11/" XCONFIGFILE);
		filename = append_version(filename);
		printf("Shall I write it to %s? ", filename);
		getstring(s);
		printf("\n");
		if (answerisyes(s))
			return filename;
#endif

		if (filename)
			free(filename);
		filename = Strdup(TREEROOTCFG "/" XCONFIGFILE);
		filename = append_version(filename);
		printf("Please answer the following question with either 'y' or 'n'.\n");
		printf("Shall I write it to the default location, %s? ", filename);
		getstring(s);
		printf("\n");
		if (answerisyes(s))
			return filename;

#ifndef PREFER_XF86CONFIG_IN_ETC
		if (filename)
			free(filename);
		filename = Strdup("/etc/X11/" XCONFIGFILE);
		filename = append_version(filename);
		printf("Shall I write it to %s? ", filename);
		getstring(s);
		printf("\n");
		if (answerisyes(s))
			return filename;
#endif
	}

	if (filename)
		free(filename);
	filename = Strdup(XCONFIGFILE);
	filename = append_version(filename);
	printf("Do you want it written to the current directory as '%s'? ", filename);
	getstring(s);
	printf("\n");
	if (answerisyes(s)) {
		return filename;
	}

	printf("Please give a filename to write to: ");
	getstring(s);
	printf("\n");
	if (filename)
		free(filename);
	filename = Strdup(s);
	return filename;
}


/*
 * Check if an earlier version of XFree86 is installed; warn about proper
 * search path order in that case.
 */

static char *notinstalled_text =
"The directory " TREEROOT " does not exist. This probably means that you have\n"
"not yet installed the version of "__XSERVERNAME__" that this program was built\n"
"to configure. Please install "__XSERVERNAME__" "XVERSIONSTRING" before running this program,\n"
"following the instructions in the INSTALL or README that comes with the\n"
__XSERVERNAME__" distribution for your OS.\n"
"For a minimal installation it is sufficient to only install base binaries,\n"
"libraries, configuration files and a server that you want to use.\n"
"\n";

static char *oldxfree86_text =
"The directory '/usr/X386/bin' exists. You probably have a very old version of\n"
"XFree86 installed, but this program was built to configure "__XSERVERNAME__" "XVERSIONSTRING"\n"
"installed in '" TREEROOT "' instead of '/usr/X386'.\n"
"\n"
"It is important that the directory '" TREEROOT "' is present in your\n"
"search path, *before* any occurrence of '/usr/X386/bin'. If you have installed\n"
"X program binaries that are not in the base "__XSERVERNAME__" distribution in\n"
"'/usr/X386/bin', you can keep the directory in your path as long as it is\n"
"after '" TREEROOT "'.\n"
"\n";

static char *pathnote_text =	
"Note that the X binary directory in your path may be a symbolic link.\n"
"In that case you could modify the symbolic link to point to the new binaries.\n"
"Example: 'rm -f /usr/bin/X11; ln -s /usr/X11R6/bin /usr/bin/X11', if the\n"
"link is '/usr/bin/X11'.\n"
"\n"
"Make sure the path is OK before continuing.\n";

static void 
path_check(void) {
	char s[80];
	int ok;

	ok = exists_dir(TREEROOT);
	if (!ok) {
		printf("%s", notinstalled_text);
		printf("Do you want to continue? ");
		getstring(s);
		if (!answerisyes(s))
			exit(-1);
		printf("\n");
	}

	ok = exists_dir("/usr/X386/bin");
	if (!ok)
		return;

	printf("%s", oldxfree86_text);
	printf("Your PATH is currently set as follows:\n%s\n\n",
		getenv("PATH"));
	printf("%s", pathnote_text);
	keypress();
}


static void
configdir_check(void)
{
	/* /etc/X11 may not exist on some systems */
	if (getuid() == 0) {
		struct stat buf;
		if (stat("/etc/X11", &buf) == -1 && errno == ENOENT)
			mkdir("/etc/X11", 0777);
		if (stat(TREEROOTCFG, &buf) == -1 && errno == ENOENT)
			mkdir(TREEROOTCFG, 0777);
	}
}


/*
 * Program entry point.
 */

int 
main(int argc, char *argv[]) {
    
	createtmpdir();

	emptylines();

	printf("%s", intro_text);

	keypress();
	emptylines();

	path_check();

	emptylines();

	configdir_check();

	emptylines();

	mouse_configuration();

	emptylines();

	keyboard_configuration();

	emptylines();

	monitor_configuration();

	emptylines();

	carddb_configuration();

	emptylines();

 	while(screen_configuration()){};

	emptylines();

	depth_configuration();

	emptylines();

	while(write_XF86Config(ask_XF86Config_location())){};

	printf("%s", finalcomment_text);

	exit(0);
}
