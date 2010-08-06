LIBRARY = libXWin

SRCS_CLIPBOARD = \
	winclipboardinit.c \
	winclipboardtextconv.c \
	winclipboardthread.c \
	winclipboardunicode.c \
	winclipboardwndproc.c \
	winclipboardwrappers.c \
	winclipboardxevents.c

DEFINES += XWIN_CLIPBOARD

SRCS_MULTIWINDOW = \
	winmultiwindowshape.c \
	winmultiwindowwindow.c \
	winmultiwindowwm.c \
	winmultiwindowwndproc.c

DEFINES += XWIN_MULTIWINDOW

#SRCS_MULTIWINDOWEXTWM = \
#	winwin32rootless.c \
#	winwin32rootlesswindow.c \
#	winwin32rootlesswndproc.c \
#	winwindowswm.c
#DEFINES += XWIN_MULTIWINDOWEXTWM

#SRCS_NATIVEGDI = \
#	winclip.c \
#	winfillsp.c \
#	winfont.c \
#	wingc.c \
#	wingetsp.c \
#	winnativegdi.c \
#	winpixmap.c \
#	winpolyline.c \
#	winrop.c \
#	winsetsp.c
#DEFINES +=  XWIN_NATIVEGDI

#SRCS_PRIMARYFB = \
#	winpfbdd.c
#DEFINES += XWIN_PRIMARYFB

SRCS_RANDR = \
	winrandr.c
DEFINES += XWIN_RANDR RELOCATE_PROJECTROOT

CSRCS =	InitInput.c \
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
	winprefslex.c \
	winprefsyacc.c \
	winwndproc.c \
	windisplay.c \
	$(SRCS_RANDR) \
	$(SRCS_CLIPBOARD) \
	$(SRCS_MULTIWINDOW) \
	$(SRCS_MULTIWINDOWEXTWM) \
	$(SRCS_NATIVEGDI) \
	$(SRCS_PRIMARYFB)


DEFINES += YY_NO_UNISTD_H

#	XWin.rc \
#	$(top_srcdir)/Xext/dpmsstubs.c \
#	$(top_srcdir)/Xi/stubs.c \
#	$(top_srcdir)/mi/miinitext.c \
#	$(top_srcdir)/fb/fbcmap_mi.c \
#	$(SRCS_PRIMARYFB) \
#	$(SRCS_XV)

INCLUDES += ..\..\miext\rootless

#.rc.o:

#	$(WINDRES) --use-temp-file -i $< --input-format=rc -o $@ -O coff -I $(top_builddir)/include -DPROJECT_NAME=\"$(VENDOR_NAME_SHORT)\" -DPROJECT_VERSION=\"$(PACKAGE_VERSION)\" -DBUILD_DATE=\"$(BUILD_DATE)\"

DEFINES += HAVE_XWIN_CONFIG_H BUILD_DATE=0-0-0000 PTW32_STATIC_LIB

#CSRCS=winkeybd.c

INCLUDES += $(OBJDIR)

$(OBJDIR)\winprefslex.c: winprefslex.l winprefsyacc.h
	flex -i -o$@ $<

$(OBJDIR)\winprefsyacc.c $(OBJDIR)\winprefsyacc.h: winprefsyacc.y
	bison -d -o$(OBJDIR)\winprefsyacc.c $<
