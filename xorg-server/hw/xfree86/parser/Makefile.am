if INSTALL_LIBXF86CONFIG
noinst_LTLIBRARIES = libxf86config_internal.la
lib_LIBRARIES = libxf86config.a
LIBHEADERS = \
	xf86Optrec.h \
	xf86Parser.h
else
noinst_LTLIBRARIES = libxf86config_internal.la
endif

INTERNAL_SOURCES= \
	Device.c \
	Files.c \
	Flags.c \
	Input.c \
	Layout.c \
	Module.c \
	Video.c \
	Monitor.c \
	Pointer.c \
	Screen.c \
	Vendor.c \
	read.c \
	scan.c \
	write.c \
	DRI.c \
	Extensions.c

libxf86config_internal_la_SOURCES = \
	$(INTERNAL_SOURCES)

libxf86config_a_SOURCES = \
	$(INTERNAL_SOURCES)
libxf86config_a_CFLAGS = $(AM_CFLAGS)

AM_CFLAGS = $(DIX_CFLAGS) $(XORG_CFLAGS)

EXTRA_DIST = \
	Configint.h \
	configProcs.h \
	xf86Optrec.h \
	xf86Parser.h \
	xf86tokens.h

sdk_HEADERS = \
	xf86Parser.h \
	xf86Optrec.h
