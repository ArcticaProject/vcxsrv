sdk_HEADERS = edid.h xf86DDC.h

noinst_LIBRARIES = libddc.a

libddc_a_SOURCES = xf86DDC.c edid.c interpret_edid.c print_edid.c \
                   ddcProperty.c

INCLUDES = $(XORG_INCS) -I$(srcdir)/../i2c

AM_CFLAGS = $(DIX_CFLAGS) $(XORG_CFLAGS)

EXTRA_DIST = ddcPriv.h DDC.HOWTO
