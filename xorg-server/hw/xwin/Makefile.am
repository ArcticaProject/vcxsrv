bin_PROGRAMS = XWin

if XWIN_CLIPBOARD
SRCS_CLIPBOARD = \
	winclipboardinit.c \
	winclipboardtextconv.c \
	winclipboardthread.c \
	winclipboardunicode.c \
	winclipboardwndproc.c \
	winclipboardwrappers.c \
	winclipboardxevents.c
DEFS_CLIPBOARD = -DXWIN_CLIPBOARD
endif

if XWIN_GLX_WINDOWS
SRCS_GLX_WINDOWS = \
	winpriv.c
DEFS_GLX_WINDOWS = -DXWIN_GLX_WINDOWS
endif

if XWIN_MULTIWINDOW
SRCS_MULTIWINDOW = \
	winmultiwindowshape.c \
	winmultiwindowwindow.c \
	winmultiwindowwm.c \
	winmultiwindowwndproc.c
DEFS_MULTIWINDOW = -DXWIN_MULTIWINDOW
endif

if XWIN_MULTIWINDOWEXTWM
SRCS_MULTIWINDOWEXTWM = \
	winwin32rootless.c \
	winwin32rootlesswindow.c \
	winwin32rootlesswndproc.c \
	winwindowswm.c
DEFS_MULTIWINDOWEXTWM = -DXWIN_MULTIWINDOWEXTWM
MULTIWINDOWEXTWM_LIBS = $(top_builddir)/miext/rootless/librootless.la
endif

if XWIN_NATIVEGDI
SRCS_NATIVEGDI = \
	winclip.c \
	winfillsp.c \
	winfont.c \
	wingc.c \
	wingetsp.c \
	winnativegdi.c \
	winpixmap.c \
	winpolyline.c \
	winrop.c \
	winsetsp.c
DEFS_NATIVEGDI = -DXWIN_NATIVEGDI
endif

if XWIN_PRIMARYFB
SRCS_PRIMARYFB = \
	winpfbdd.c
DEFS_PRIMARYFB = -DXWIN_PRIMARYFB
endif

if XWIN_RANDR
SRCS_RANDR = \
	winrandr.c
DEFS_RANDR = -DXWIN_RANDR
endif

if XWIN_XV
SRCS_XV = \
	winvideo.c
DEFS_XV = -DXWIN_XV
endif

SRCS =	InitInput.c \
	InitOutput.c \
	winallpriv.c \
	winauth.c \
	winblock.c \
	wincmap.c \
	winconfig.c \
	wincreatewnd.c \
	wincursor.c \
	windialogs.c \
	winengine.c \
	winerror.c \
	winglobals.c \
	winkeybd.c \
	winkeyhook.c \
	winmisc.c \
	winmouse.c \
	winmsg.c \
	winmultiwindowclass.c \
	winmultiwindowicons.c \
	winprefs.c \
	winprefsyacc.y \
	winprefslex.l \
	winprocarg.c \
	winregistry.c \
	winscrinit.c \
	winshaddd.c \
	winshadddnl.c \
	winshadgdi.c \
	wintrayicon.c \
	winvalargs.c \
	winwakeup.c \
	winwindow.c \
	winwndproc.c \
	ddraw.h \
	winclipboard.h \
	winconfig.h \
	win.h \
	winkeybd.h \
	winkeynames.h \
	winlayouts.h \
	winmessages.h \
	winmsg.h \
	winms.h \
	winmultiwindowclass.h \
	winprefs.h \
	winpriv.h \
	winresource.h \
	winwindow.h \
	XWin.rc \
	$(top_srcdir)/Xext/dpmsstubs.c \
	$(top_srcdir)/Xi/stubs.c \
	$(top_srcdir)/mi/miinitext.c \
	$(top_srcdir)/fb/fbcmap_mi.c \
	$(SRCS_CLIPBOARD) \
	$(SRCS_GLX_WINDOWS) \
	$(SRCS_MULTIWINDOW) \
	$(SRCS_MULTIWINDOWEXTWM) \
	$(SRCS_NATIVEGDI) \
	$(SRCS_PRIMARYFB) \
	$(SRCS_RANDR) \
	$(SRCS_XV)

 DEFS = $(DEFS_CLIPBOARD) \
	$(DEFS_GLX_WINDOWS) \
	$(DEFS_MULTIWINDOW) \
	$(DEFS_MULTIWINDOWEXTWM) \
	$(DEFS_NATIVEGDI) \
	$(DEFS_PRIMARYFB) \
	$(DEFS_RANDR) \
	$(DEFS_XV)

XWin_SOURCES = $(SRCS)

INCLUDES = -I$(top_srcdir)/miext/rootless

XWin_DEPENDENCIES = $(XWIN_LIBS)
XWin_LDADD = $(MULTIWINDOWEXTWM_LIBS) $(XWIN_LIBS) $(MAIN_LIB) $(XSERVER_LIBS) $(XSERVER_SYS_LIBS) $(XWIN_SYS_LIBS)

.rc.o:
	$(AM_V_GEN)$(WINDRES) --use-temp-file -i $< --input-format=rc -o $@ -O coff -I $(top_builddir)/include

XWin_LDFLAGS = -mwindows -static

winprefsyacc.h: winprefsyacc.c
winprefslex.c: winprefslex.l winprefsyacc.c winprefsyacc.h

BUILT_SOURCES = winprefsyacc.h winprefsyacc.c winprefslex.c
CLEANFILES = $(BUILT_SOURCES) $(appman_DATA) $(fileman_DATA) XWin.man XWinrc.man

AM_YFLAGS = -d
AM_LFLAGS = -i
AM_CFLAGS = -DHAVE_XWIN_CONFIG_H $(DIX_CFLAGS) \
            $(XWINMODULES_CFLAGS) \
            -DXFree86Server

GLX_EXTRAS = \
	glx/glwindows.h \
	glx/glwrap.c \
	glx/indirect.c

MAN_SRCS = XWin.man.pre XWinrc.man.pre

appmandir = $(APP_MAN_DIR)
appman_DATA = XWin.$(APP_MAN_SUFFIX)

filemandir = $(FILE_MAN_DIR)
fileman_DATA = XWinrc.$(FILE_MAN_SUFFIX)

XWin.$(APP_MAN_SUFFIX): XWin.man
	-$(AM_V_at)rm -f XWin.$(APP_MAN_SUFFIX)
	$(AM_V_at)$(LN_S) XWin.man XWin.$(APP_MAN_SUFFIX)

XWinrc.$(FILE_MAN_SUFFIX): XWinrc.man
	-$(AM_V_at)rm -f XWinrc.$(FILE_MAN_SUFFIX)
	$(AM_V_at)$(LN_S) XWinrc.man XWinrc.$(FILE_MAN_SUFFIX)

EXTRAMANDEFS = -D__logdir__=$(logdir) -D__sysconfdir__=$(sysconfdir) -D__datadir__=$(datadir)

xwinconfigdir = $(sysconfdir)/X11
xwinconfig_DATA = system.XWinrc

include $(top_srcdir)/cpprules.in

install-exec-hook:
	(cd $(DESTDIR)$(bindir) && rm -f X && $(LN_S) XWin$(EXEEXT) X)

EXTRA_DIST = \
	$(GLX_EXTRAS) \
	$(MAN_SRCS) \
	$(xwinconfig_DATA) \
	X.ico \
	XWin.rc \
	xlaunch/config.cc \
	xlaunch/COPYING \
	xlaunch/main.cc \
	xlaunch/resources/dialog.rc \
	xlaunch/resources/fullscreen.bmp \
	xlaunch/resources/images.rc \
	xlaunch/resources/multiwindow.bmp \
	xlaunch/resources/nodecoration.bmp \
	xlaunch/resources/resources.h \
	xlaunch/resources/resources.rc \
	xlaunch/resources/strings.rc \
	xlaunch/resources/windowed.bmp \
	xlaunch/window/dialog.cc \
	xlaunch/window/dialog.h \
	xlaunch/window/util.cc \
	xlaunch/window/util.h \
	xlaunch/window/window.cc \
	xlaunch/window/window.h \
	xlaunch/window/wizard.cc \
	xlaunch/window/wizard.h

relink:
	$(AM_V_at)rm -f XWin$(EXEEXT) && $(MAKE) XWin$(EXEEXT)
