DIST_SUBDIRS = input config glxProxy examples doc

SUBDIRS = input config examples
bin_PROGRAMS = Xdmx

if XINERAMA
PANORAMIX_SRCS = $(top_srcdir)/Xext/panoramiX.c
endif

if GLX
SUBDIRS += glxProxy
GLX_LIBS = glxProxy/libglxproxy.a
GLX_SRCS = $(PANORAMIX_SRCS) dmx_glxvisuals.c dmx_glxvisuals.h
GLX_INCS = -I$(top_srcdir)/hw/xfree86/dixmods/extmod
GLX_DEFS = @GL_CFLAGS@
endif

if BUILDDOCS
SUBDIRS += doc
endif

AM_CFLAGS = \
      -DHAVE_DMX_CONFIG_H \
      $(DIX_CFLAGS) \
      $(GLX_INCS) \
      $(GLX_DEFS) \
      $(DMX_CFLAGS) \
      @DMXMODULES_CFLAGS@

Xdmx_SOURCES = dmx.c \
               dmxcb.c \
               dmxcb.h \
               dmxclient.h \
               dmxcmap.c \
               dmxcmap.h \
               dmx-config.h \
               dmxcursor.c \
               dmxcursor.h \
               dmxdpms.c \
               dmxdpms.h \
               dmxextension.c \
               dmxextension.h \
               dmxfont.c \
               dmxfont.h \
               dmxgc.c \
               dmxgc.h \
               dmxgcops.c \
               dmxgcops.h \
               dmx.h \
               dmxinit.c \
               dmxinit.h \
               dmxinput.c \
               dmxinput.h \
               dmxlog.c \
               dmxlog.h \
               dmxpict.c \
               dmxpict.h \
               dmxpixmap.c \
               dmxpixmap.h \
               dmxprop.c \
               dmxprop.h \
               dmxscrinit.c \
               dmxscrinit.h \
               dmxshadow.c \
               dmxshadow.h \
               dmxstat.c \
               dmxstat.h \
               dmxsync.c \
               dmxsync.h \
               dmxvisual.c \
               dmxvisual.h \
               dmxwindow.c \
               dmxwindow.h \
               $(top_srcdir)/mi/miinitext.c \
               $(top_srcdir)/fb/fbcmap_mi.c \
               $(GLX_SRCS) 


#if COMPOSITE
#Xdmx_SOURCES += fakecw.c
#endif

XDMX_LIBS = \
	@XDMX_LIBS@ \
	$(GLX_LIBS) \
        input/libdmxinput.a \
        config/libdmxconfig.a

Xdmx_LDFLAGS = $(LD_EXPORT_SYMBOLS_FLAG)
Xdmx_DEPENDENCIES= $(XDMX_LIBS)
Xdmx_LDADD = $(XDMX_LIBS) $(XDMX_SYS_LIBS) $(XSERVER_SYS_LIBS)

# Man page
appmandir = $(APP_MAN_DIR)

appman_PRE = Xdmx.man
appman_DATA = $(appman_PRE:man=@APP_MAN_SUFFIX@)

CLEANFILES = $(appman_DATA)

# Strings to replace in man pages
XORGRELSTRING = @PACKAGE_STRING@
  XORGMANNAME = X Version 11

MAN_SUBSTS = \
	-e 's|__vendorversion__|"$(XORGRELSTRING)" "$(XORGMANNAME)"|' \
	-e 's|__miscmansuffix__|$(MISC_MAN_SUFFIX)|g'

SUFFIXES = .$(APP_MAN_SUFFIX) .man

.man.$(APP_MAN_SUFFIX):
	$(AM_V_GEN)$(SED) $(MAN_SUBSTS) < $< > $@

EXTRA_DIST = $(appman_PRE)

relink:
	$(AM_V_at)rm -f Xdmx$(EXEEXT) && $(MAKE) Xdmx$(EXEEXT)
