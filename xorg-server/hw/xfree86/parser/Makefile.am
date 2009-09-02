if INSTALL_LIBXF86CONFIG
lib_LIBRARIES = libxf86config.a
else
noinst_LIBRARIES = libxf86config.a
endif

libxf86config_a_SOURCES = \
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

AM_CFLAGS = $(DIX_CFLAGS) $(XORG_CFLAGS)

EXTRA_DIST = \
	Configint.h \
	configProcs.h \
	xf86tokens.h

sdk_HEADERS = \
	xf86Optrec.h \
	xf86Parser.h
