SUBDIRS = man

module_LTLIBRARIES = libfbdevhw.la

libfbdevhw_la_LDFLAGS = -avoid-version

if FBDEVHW
libfbdevhw_la_SOURCES = fbdevhw.c
else
libfbdevhw_la_SOURCES = fbdevhwstub.c
endif

INCLUDES = $(XORG_INCS) -I$(srcdir)/../i2c

AM_CFLAGS = $(DIX_CFLAGS) $(XORG_CFLAGS)

sdk_HEADERS = fbdevhw.h

EXTRA_DIST = fbpriv.h README
