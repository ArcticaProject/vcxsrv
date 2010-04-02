noinst_LIBRARIES = libdmxconfig.a

LIBSRCS = parser.y \
          scanner.l \
          dmxparse.c \
          dmxparse.h \
          dmxprint.c \
          dmxprint.h \
          dmxcompat.c \
          dmxcompat.h \
          dmxconfig.c \
          dmxconfig.h

parser.h: parser.c
scanner.c: scanner.l parser.h

BUILT_SOURCES = parser.c parser.h scanner.c

MAINTAINERCLEANFILES = $(BUILT_SOURCES)

libdmxconfig_a_SOURCES = $(LIBSRCS)

if GLX
GLX_DEFS = @GL_CFLAGS@
endif	    

AM_YFLAGS = -d
AM_CFLAGS = \
	$(DIX_CFLAGS) \
	-I$(top_srcdir)/hw/dmx \
	-DHAVE_DMX_CONFIG_H \
	-DDMX_LOG_STANDALONE \
	$(GLX_DEFS) \
	@DMXMODULES_CFLAGS@

bin_PROGRAMS = xdmxconfig vdltodmx dmxtodmx

xdmxconfig_DEPENDENCIES = libdmxconfig.a
xdmxconfig_SOURCES = \
	xdmxconfig.c \
	$(top_srcdir)/hw/dmx/dmxlog.c \
	Canvas.c \
	Canvas.h \
	CanvasP.h
xdmxconfig_LDADD = -L. -ldmxconfig @XDMXCONFIG_DEP_LIBS@
xdmxconfig_CFLAGS = $(AM_CFLAGS) @XDMXCONFIG_DEP_CFLAGS@

vdltodmx_DEPENDENCIES = libdmxconfig.a
vdltodmx_SOURCES = vdltodmx.c
vdltodmx_LDADD = -L. -ldmxconfig

dmxtodmx_DEPENDENCIES = libdmxconfig.a
dmxtodmx_SOURCES = dmxtodmx.c
dmxtodmx_LDADD = -L. -ldmxconfig

appmandir = $(APP_MAN_DIR)

appman_PRE = xdmxconfig.man vdltodmx.man dmxtodmx.man
appman_DATA = $(appman_PRE:man=@APP_MAN_SUFFIX@)

CLEANFILES = $(appman_DATA)

SUFFIXES = .$(APP_MAN_SUFFIX) .man

# Strings to replace in man pages
XORGRELSTRING = @PACKAGE_STRING@
  XORGMANNAME = X Version 11

MAN_SUBSTS = -e 's|__vendorversion__|"$(XORGRELSTRING)" "$(XORGMANNAME)"|' 

.man.$(APP_MAN_SUFFIX):
	$(AM_V_GEN)$(SED) $(MAN_SUBSTS) < $< > $@

EXTRA_DIST = \
	$(appman_PRE) \
	test-a.in test-a.out \
	test-b.in test-b.out \
	test-c.in test-c.out \
	test-d.in test-d.out \
	test-e.in test-e.out \
	test-f.in test-f.out \
	test-g.in test-g.out \
	test-h.in test-h.out \
	test-i.in test-i.out \
	test-j.in test-j.out \
	test-k.in test-k.out \
	test-l.in test-l.out
