module_LTLIBRARIES = libxf8_16bpp.la

sdk_HEADERS = cfb8_16.h

INCLUDES = $(XORG_INCS) -I$(top_srcdir)/fb

AM_CFLAGS = $(DIX_CFLAGS) $(XORG_CFLAGS)

libxf8_16bpp_la_LDFLAGS = -avoid-version

libxf8_16bpp_la_SOURCES = xf8_16module.c
