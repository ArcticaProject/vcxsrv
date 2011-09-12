if ENABLE_UNIT_TESTS
if HAVE_LD_WRAP
SUBDIRS= . xi2
noinst_PROGRAMS = xkb input xtest list misc fixes
check_LTLIBRARIES = libxservertest.la

TESTS=$(noinst_PROGRAMS)

AM_CFLAGS = $(DIX_CFLAGS) @XORG_CFLAGS@
INCLUDES = $(XORG_INCS) -I$(top_srcdir)/hw/xfree86/parser \
	-I$(top_srcdir)/miext/cw -I$(top_srcdir)/hw/xfree86/ddc \
	-I$(top_srcdir)/hw/xfree86/i2c -I$(top_srcdir)/hw/xfree86/modes \
	-I$(top_srcdir)/hw/xfree86/ramdac
TEST_LDADD=libxservertest.la $(XORG_SYS_LIBS) $(XSERVER_SYS_LIBS)

if SPECIAL_DTRACE_OBJECTS
TEST_LDADD += $(OS_LIB) $(DIX_LIB)
endif

xkb_LDADD=$(TEST_LDADD)
input_LDADD=$(TEST_LDADD)
xtest_LDADD=$(TEST_LDADD)
list_LDADD=$(TEST_LDADD)
misc_LDADD=$(TEST_LDADD)
fixes_LDADD=$(TEST_LDADD)

nodist_libxservertest_la_SOURCES = $(top_builddir)/hw/xfree86/sdksyms.c
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
            $(top_builddir)/mi/libmi.la \
            $(top_builddir)/os/libos.la \
            @XORG_LIBS@
libxservertest_la_DEPENDENCIES = $(libxservertest_la_LIBADD)
endif
endif
