bin_PROGRAMS = Xvfb
noinst_LIBRARIES = libfbcmap.a

AM_CFLAGS = -DHAVE_DIX_CONFIG_H \
            -DNO_HW_ONLY_EXTS \
            -DNO_MODULE_EXTS \
            $(XVFBMODULES_CFLAGS) \
	    $(DIX_CFLAGS)

SRCS =	InitInput.c \
	InitOutput.c \
	lk201kbd.h \
	$(top_srcdir)/Xext/dpmsstubs.c \
	$(top_srcdir)/Xi/stubs.c \
	$(top_srcdir)/mi/miinitext.c

libfbcmap_a_CFLAGS = $(AM_CFLAGS)
libfbcmap_a_SOURCES = $(top_srcdir)/fb/fbcmap_mi.c

Xvfb_SOURCES = $(SRCS)

XVFB_LIBS = \
        @XVFB_LIBS@ \
	libfbcmap.a \
	$(MAIN_LIB) \
	$(XSERVER_LIBS)

Xvfb_LDADD = $(XVFB_LIBS) $(XVFB_SYS_LIBS) $(XSERVER_SYS_LIBS)
Xvfb_DEPENDENCIES = $(XVFB_LIBS)
Xvfb_LDFLAGS = $(LD_EXPORT_SYMBOLS_FLAG)

# Man page
include $(top_srcdir)/cpprules.in

appmandir = $(APP_MAN_DIR)

appman_PRE = Xvfb.man
appman_DATA = $(appman_PRE:man=@APP_MAN_SUFFIX@)

BUILT_SOURCES = $(appman_PRE)
CLEANFILES = $(appman_PRE) $(appman_DATA)

SUFFIXES += .$(APP_MAN_SUFFIX) .man

.man.$(APP_MAN_SUFFIX):
	-rm -f $@
	$(LN_S) $< $@

EXTRA_DIST = Xvfb.man.pre

relink:
	rm -f Xvfb && $(MAKE) Xvfb
