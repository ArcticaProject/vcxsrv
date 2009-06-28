module_LTLIBRARIES = libvbe.la
libvbe_la_LDFLAGS = -avoid-version
libvbe_la_SOURCES = vbe.c vbeModes.c vbe_module.c

sdk_HEADERS = vbe.h vbeModes.h

AM_CFLAGS = $(DIX_CFLAGS) $(XORG_CFLAGS)

INCLUDES = $(XORG_INCS) -I$(srcdir)/../ddc -I$(srcdir)/../i2c \
           -I$(srcdir)/../int10
