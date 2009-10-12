if UNITTESTS
SUBDIRS= . xi2
check_PROGRAMS = xkb input xtest
check_LTLIBRARIES = libxservertest.la

TESTS=$(check_PROGRAMS)

AM_CFLAGS = $(DIX_CFLAGS) $(GLIB_CFLAGS) @XORG_CFLAGS@
INCLUDES = @XORG_INCS@
TEST_LDADD=libxservertest.la $(XORG_SYS_LIBS) $(XSERVER_SYS_LIBS) $(GLIB_LIBS)

if SPECIAL_DTRACE_OBJECTS
TEST_LDADD += $(OS_LIB) $(DIX_LIB)
endif

xkb_LDADD=$(TEST_LDADD)
input_LDADD=$(TEST_LDADD)
xtest_LDADD=$(TEST_LDADD)

libxservertest_la_LIBADD = \
            $(XSERVER_LIBS) \
            $(top_builddir)/hw/xfree86/loader/libloader.la \
            $(top_builddir)/hw/xfree86/os-support/libxorgos.la \
            $(top_builddir)/hw/xfree86/common/libcommon.la \
            $(top_builddir)/hw/xfree86/parser/libxf86config_internal.la \
            $(top_builddir)/hw/xfree86/dixmods/libdixmods.la \
            $(top_builddir)/hw/xfree86/modes/libxf86modes.la \
            $(top_builddir)/hw/xfree86/ramdac/libramdac.la \
            $(top_builddir)/hw/xfree86/ddc/libddc.la \
            $(top_builddir)/hw/xfree86/i2c/libi2c.la \
            $(top_builddir)/hw/xfree86/dixmods/libxorgxkb.la \
            $(top_builddir)/hw/xfree86/libxorg.la \
            $(top_builddir)/mi/libmi.la \
            $(top_builddir)/os/libos.la \
            @XORG_LIBS@

CLEANFILES=libxservertest.c

libxservertest.c:
	touch $@

endif
