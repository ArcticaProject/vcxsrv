noinst_LTLIBRARIES = libXquartz.la
AM_CFLAGS = $(XSERVER_CFLAGS) $(DIX_CFLAGS)
AM_OBJCFLAGS = $(XSERVER_CFLAGS) $(DIX_CFLAGS)
AM_CPPFLAGS = \
	-DBUILD_DATE=\"$(BUILD_DATE)\" \
	-DXSERVER_VERSION=\"$(VERSION)\" \
	-DINXQUARTZ \
	-DUSE_NEW_CLUT \
	-DXFree86Server \
	-I$(top_srcdir)/miext/rootless \
	-DX11LIBDIR=\"$(libdir)\"

if GLX
GL_DIR = GL
endif

SUBDIRS = bundle . $(GL_DIR) xpr pbproxy mach-startup doc

DIST_SUBDIRS = bundle . GL xpr pbproxy mach-startup doc

libXquartz_la_SOURCES = \
	$(top_srcdir)/fb/fbcmap_mi.c \
	$(top_srcdir)/mi/miinitext.c \
	X11Application.m \
	X11Controller.m \
	applewm.c \
	darwin.c \
	darwinEvents.c \
	darwinXinput.c \
	keysym2ucs.c \
	pseudoramiX.c \
	quartz.c \
	quartzAudio.c \
	quartzCocoa.m \
	quartzKeyboard.c \
	quartzStartup.c \
	threadSafety.c

EXTRA_DIST = \
	X11Application.h \
	X11Controller.h \
	applewmExt.h \
	darwin.h \
	darwinfb.h \
	darwinEvents.h \
	keysym2ucs.h \
	pseudoramiX.h \
	quartz.h \
	quartzAudio.h \
	quartzCommon.h \
	quartzKeyboard.h \
	sanitizedCarbon.h \
	sanitizedCocoa.h \
	threadSafety.h
