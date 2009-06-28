module_LTLIBRARIES = libxf8_32bpp.la

sdk_HEADERS = cfb8_32.h

INCLUDES = $(XORG_INCS) -I$(top_srcdir)/mfb -I$(top_srcdir)/cfb

AM_CFLAGS = $(DIX_CFLAGS) $(XORG_CFLAGS)

libxf8_32bpp_la_LDFLAGS = -avoid-version

libxf8_32bpp_la_SOURCES = \
	cfbcpyarea.c \
	cfbcpyplane.c \
	cfbgcmisc.c \
	cfbimage.c \
	cfbpntwin.c \
	cfbscrinit.c \
	cfbwindow.c \
	xf86overlay.c \
	cfb8_32module.c \
	cfbgc8.c \
	cfbgc32.c \
	cfbgcunder.c

libxf8_32bpp_la_LIBADD = $(top_builddir)/cfb32/libcfb32.la

EXTRA_DIST = cfbgc.c

cfbgc8.c: $(srcdir)/cfbgc.c
	echo '#define PSZ 8' > $@
	echo '#include "$(srcdir)/cfbgc.c"' >> $@

cfbgc32.c: $(srcdir)/cfbgc.c
	echo '#define PSZ 32' > $@
	echo '#include "$(srcdir)/cfbgc.c"' >> $@

DISTCLEANFILES = cfbgc8.c cfbgc32.c
