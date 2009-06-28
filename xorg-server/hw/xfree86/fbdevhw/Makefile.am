linuxmoduledir = $(moduledir)/linux
linuxmodule_LTLIBRARIES = libfbdevhw.la

libfbdevhw_la_LDFLAGS = -avoid-version

if FBDEVHW
libfbdevhw_la_SOURCES = fbdevhw.c
else
libfbdevhw_la_SOURCES = fbdevhwstub.c
endif

INCLUDES = $(XORG_INCS) -I$(srcdir)/../i2c

AM_CFLAGS = $(DIX_CFLAGS) $(XORG_CFLAGS)

sdk_HEADERS = fbdevhw.h

include $(top_srcdir)/cpprules.in

drivermandir = $(DRIVER_MAN_DIR)
driverman_DATA = fbdevhw.$(DRIVER_MAN_SUFFIX)
CLEANFILES = $(driverman_DATA) fbdevhw.man

fbdevhw.$(DRIVER_MAN_SUFFIX): fbdevhw.man
	-rm -f fbdevhw.$(DRIVER_MAN_SUFFIX)
	$(LN_S) fbdevhw.man fbdevhw.$(DRIVER_MAN_SUFFIX)

EXTRA_DIST = fbpriv.h fbdevhw.man.pre README
