AM_CPPFLAGS = \
	-DBUILD_DATE=\"$(BUILD_DATE)\" \
	-DXSERVER_VERSION=\"$(VERSION)\" \
	-DX11BINDIR=\"$(bindir)\"

x11appdir = $(APPLE_APPLICATIONS_DIR)/X11.app/Contents/MacOS
x11app_PROGRAMS = X11.bin

dist_X11_bin_SOURCES = \
	bundle-main.c 

nodist_X11_bin_SOURCES = \
	mach_startupServer.c \
	mach_startupUser.c

X11_bin_LDADD = \
	$(top_builddir)/hw/xquartz/libXquartz.la \
	$(top_builddir)/hw/xquartz/xpr/libXquartzXpr.la \
	$(top_builddir)/dix/dixfonts.lo \
	$(top_builddir)/miext/rootless/librootless.la \
	$(top_builddir)/hw/xquartz/pbproxy/libxpbproxy.la \
	$(DARWIN_LIBS) $(XSERVER_LIBS) $(XSERVER_SYS_LIBS) -lXplugin

X11_bin_LDFLAGS =  \
	-XCClinker -Objc \
	-Wl,-u,_miDCInitialize \
	-Wl,-framework,Carbon \
	-Wl,-framework,Cocoa \
	-Wl,-framework,CoreAudio \
	-Wl,-framework,IOKit

if GLX
X11_bin_LDADD += \
	$(top_builddir)/hw/xquartz/GL/libCGLCore.la \
	$(top_builddir)/glx/libglx.la

X11_bin_LDFLAGS += \
	-L/System/Library/Frameworks/OpenGL.framework/Libraries -lGL \
	-Wl,-framework,OpenGL
endif

if RECORD
X11_bin_LDADD += \
	$(top_builddir)/record/librecord.la
endif

bin_PROGRAMS = Xquartz

dist_Xquartz_SOURCES = \
	stub.c \
	launchd_fd.c

nodist_Xquartz_SOURCES = \
	mach_startupUser.c

Xquartz_LDFLAGS =  \
	-Wl,-framework,CoreServices

BUILT_SOURCES = \
	mach_startupServer.c \
	mach_startupUser.c \
	mach_startupServer.h \
	mach_startup.h

CLEANFILES = \
	$(BUILT_SOURCES)

$(BUILT_SOURCES): $(srcdir)/mach_startup.defs
	mig -sheader mach_startupServer.h $(srcdir)/mach_startup.defs

EXTRA_DIST = \
	launchd_fd.h \
	mach_startup.defs \
	mach_startup_types.h
