lib_LTLIBRARIES = libXdmcp.la

AM_CFLAGS=-I${top_srcdir}/include $(XDMCP_CFLAGS)
libXdmcp_la_LDFLAGS = -version-number 6:0:0 -no-undefined
libXdmcp_la_LIBADD = $(XDMCP_LIBS)
libXdmcp_la_SOURCES = \
	A8Eq.c \
	AA16.c \
	AA32.c \
	AA8.c \
	Alloc.c \
	AofA8.c \
	CA8.c \
	CmpKey.c \
	DA16.c \
	DA32.c \
	DA8.c \
	DAofA8.c \
	DecKey.c \
	Fill.c \
	Flush.c \
	GenKey.c \
	IncKey.c \
	RA16.c \
	RA32.c \
	RA8.c \
	RAofA8.c \
	RC16.c \
	RC32.c \
	RC8.c \
	RHead.c \
	RR.c \
	RaA16.c \
	RaA32.c \
	RaA8.c \
	RaAoA8.c \
	Unwrap.c \
	WA16.c \
	WA32.c \
	WA8.c \
	WAofA8.c \
	WC16.c \
	WC32.c \
	WC8.c \
	Whead.c \
	Wrap.c \
	Wrap.h

if HASXDMAUTH
nodist_libXdmcp_la_SOURCES = Wraphelp.c
endif HASXDMAUTH

xdmcpincludedir=$(includedir)/X11

INCLUDES = \
	-I${top_builddir}/include

xdmcpinclude_HEADERS = \
	include/X11/Xdmcp.h

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = xdmcp.pc

EXTRA_DIST = xdmcp.pc.in autogen.sh Wraphelp.c Wraphelp.README.crypto

if LINT
ALL_LINT_FLAGS=$(LINT_FLAGS) $(DEFS) $(DEFAULT_INCLUDES) $(INCLUDES) \
		$(AM_CPPFLAGS) $(CPPFLAGS) $(AM_CFLAGS)

lint:
	$(LINT) $(ALL_LINT_FLAGS) $(libXdmcp_la_SOURCES) $(nodist_libXdmcp_la_SOURCES) $(LIBS) $(XDMCP_LIBS)
endif LINT

if MAKE_LINT_LIB
lintlibdir = $(libdir)

lintlib_DATA = $(LINTLIB)

$(LINTLIB): $(libXdmcp_la_SOURCES) $(nodist_libXdmcp_la_SOURCES)
	$(LINT) -y -oXdmcp -x $(ALL_LINT_FLAGS) $(libXdmcp_la_SOURCES) $(nodist_libXdmcp_la_SOURCES)
endif MAKE_LINT_LIB

if MAINTAINER_MODE
.PHONY: ChangeLog

ChangeLog:
	GIT_DIR=${srcdir}/.git git-log > ChangeLog

dist-hook: ChangeLog
endif MAINTAINER_MODE

CLEANFILES=$(lintlib_DATA)
