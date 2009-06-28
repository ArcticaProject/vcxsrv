lib_LTLIBRARIES = libxcb.la \
                  libxcb-xlib.la \
                  libxcb-composite.la \
                  libxcb-damage.la \
                  libxcb-dpms.la \
                  libxcb-glx.la \
                  libxcb-randr.la \
                  libxcb-record.la \
                  libxcb-render.la \
                  libxcb-res.la \
                  libxcb-screensaver.la \
                  libxcb-shape.la \
                  libxcb-shm.la \
                  libxcb-sync.la \
                  libxcb-xevie.la \
                  libxcb-xf86dri.la \
                  libxcb-xfixes.la \
		  libxcb-xinerama.la \
                  libxcb-xprint.la \
                  libxcb-xtest.la \
                  libxcb-xv.la \
                  libxcb-xvmc.la

EXTHEADERS =	xproto.h \
		bigreq.h \
		composite.h \
		damage.h \
		dpms.h \
		glx.h \
		randr.h \
		record.h \
		render.h \
		res.h \
		screensaver.h \
		shape.h \
		shm.h \
		sync.h \
		xc_misc.h \
		xevie.h \
		xf86dri.h \
		xfixes.h \
		xinerama.h \
		xprint.h \
		xtest.h \
		xv.h \
		xvmc.h
EXTSOURCES =	xproto.c \
		bigreq.c \
		composite.c \
		damage.c \
		dpms.c \
		glx.c \
		randr.c \
		record.c \
		render.c \
		res.c \
		screensaver.c \
		shape.c \
		shm.c \
		sync.c \
		xc_misc.c \
		xevie.c \
		xf86dri.c \
		xfixes.c \
		xinerama.c \
		xprint.c \
		xtest.c \
		xv.c \
		xvmc.c
EXTENSION_XML = xproto.xml \
		bigreq.xml \
		composite.xml \
		damage.xml \
		dpms.xml \
		glx.xml \
		randr.xml \
		record.xml \
		render.xml \
		res.xml \
		screensaver.xml \
		shape.xml \
		shm.xml \
		sync.xml \
		xc_misc.xml \
		xevie.xml \
		xf86dri.xml \
		xfixes.xml \
		xinerama.xml \
		xprint.xml \
		xtest.xml \
		xvmc.xml \
		xv.xml

xcbinclude_HEADERS = xcb.h xcbext.h xcbxlib.h $(EXTHEADERS)
noinst_HEADERS = xcbint.h

AM_CFLAGS = $(CWARNFLAGS) $(NEEDED_CFLAGS) $(XDMCP_CFLAGS)
libxcb_la_LIBADD = $(NEEDED_LIBS) $(XDMCP_LIBS)
libxcb_la_SOURCES = \
		xcb_conn.c xcb_out.c xcb_in.c xcb_ext.c xcb_xid.c \
		xcb_list.c xcb_util.c xcb_auth.c c-client.xsl \
		xproto.c bigreq.c xc_misc.c

# Explanation for -version-info:
# -version-info current:revision:age
# The library supports interface (current-age) to interface current.
# * If you changed the source code at all, increment revision.
# * If you add an interface, increment current and age and set revision to 0.
# * If you change or remove an interface, increment current and set revision
#   and age to 0.
libxcb_la_LDFLAGS = -version-info 1:0:0

BUILT_SOURCES = $(EXTSOURCES) $(EXTHEADERS)
CLEANFILES = $(EXTSOURCES) $(EXTHEADERS) $(EXTENSION_XML)

XCB_LIBS = libxcb.la

libxcb_xlib_la_LDFLAGS = -version-info 0:0:0
libxcb_xlib_la_LIBADD = $(XCB_LIBS)
libxcb_xlib_la_SOURCES = xcb_xlib.c

# FIXME: find a way to autogenerate this from the XML files.

libxcb_composite_la_LDFLAGS = -version-info 0:0:0
libxcb_composite_la_LIBADD = $(XCB_LIBS)
libxcb_composite_la_SOURCES = composite.c composite.h

libxcb_damage_la_LDFLAGS = -version-info 0:0:0
libxcb_damage_la_LIBADD = $(XCB_LIBS)
libxcb_damage_la_SOURCES = damage.c damage.h

libxcb_dpms_la_LDFLAGS = -version-info 0:0:0
libxcb_dpms_la_LIBADD = $(XCB_LIBS)
libxcb_dpms_la_SOURCES = dpms.c dpms.h

libxcb_glx_la_LDFLAGS = -version-info 0:0:0
libxcb_glx_la_LIBADD = $(XCB_LIBS)
libxcb_glx_la_SOURCES = glx.c glx.h

libxcb_randr_la_LDFLAGS = -version-info 0:0:0
libxcb_randr_la_LIBADD = $(XCB_LIBS)
libxcb_randr_la_SOURCES = randr.c randr.h

libxcb_record_la_LDFLAGS = -version-info 0:0:0
libxcb_record_la_LIBADD = $(XCB_LIBS)
libxcb_record_la_SOURCES = record.c record.h

libxcb_render_la_LDFLAGS = -version-info 0:0:0
libxcb_render_la_LIBADD = $(XCB_LIBS)
libxcb_render_la_SOURCES = render.c render.h

libxcb_res_la_LDFLAGS = -version-info 0:0:0
libxcb_res_la_LIBADD = $(XCB_LIBS)
libxcb_res_la_SOURCES = res.c res.h

libxcb_screensaver_la_LDFLAGS = -version-info 0:0:0
libxcb_screensaver_la_LIBADD = $(XCB_LIBS)
libxcb_screensaver_la_SOURCES = screensaver.c screensaver.h

libxcb_shape_la_LDFLAGS = -version-info 0:0:0
libxcb_shape_la_LIBADD = $(XCB_LIBS)
libxcb_shape_la_SOURCES = shape.c shape.h

libxcb_shm_la_LDFLAGS = -version-info 0:0:0
libxcb_shm_la_LIBADD = $(XCB_LIBS)
libxcb_shm_la_SOURCES = shm.c shm.h

libxcb_sync_la_LDFLAGS = -version-info 0:0:0
libxcb_sync_la_LIBADD = $(XCB_LIBS)
libxcb_sync_la_SOURCES = sync.c sync.h

libxcb_xevie_la_LDFLAGS = -version-info 0:0:0
libxcb_xevie_la_LIBADD = $(XCB_LIBS)
libxcb_xevie_la_SOURCES = xevie.c xevie.h

libxcb_xf86dri_la_LDFLAGS = -version-info 0:0:0
libxcb_xf86dri_la_LIBADD = $(XCB_LIBS)
libxcb_xf86dri_la_SOURCES = xf86dri.c xf86dri.h

libxcb_xfixes_la_LDFLAGS = -version-info 0:0:0
libxcb_xfixes_la_LIBADD = $(XCB_LIBS)
libxcb_xfixes_la_SOURCES = xfixes.c xfixes.h

libxcb_xprint_la_LDFLAGS = -version-info 0:0:0
libxcb_xprint_la_LIBADD = $(XCB_LIBS)
libxcb_xprint_la_SOURCES = xprint.c xprint.h

libxcb_xtest_la_LDFLAGS = -version-info 0:0:0
libxcb_xtest_la_LIBADD = $(XCB_LIBS)
libxcb_xtest_la_SOURCES = xtest.c xtest.h

libxcb_xv_la_LDFLAGS = -version-info 0:0:0
libxcb_xv_la_LIBADD = $(XCB_LIBS)
libxcb_xv_la_SOURCES = xv.c xv.h

libxcb_xvmc_la_LDFLAGS = -version-info 0:0:0
libxcb_xvmc_la_LIBADD = $(XCB_LIBS)
libxcb_xvmc_la_SOURCES = xvmc.c xvmc.h

libxcb_xinerama_la_LDFLAGS = -version-info 0:0:0
libxcb_xinerama_la_LIBADD = $(XCB_LIBS)
libxcb_xinerama_la_SOURCES = xinerama.c xinerama.h

$(EXTHEADERS) $(EXTSOURCES): c-client.xsl

SUFFIXES = .xml

.xml.h:
	@n=`dirname $*`; test -d $$n || (echo mkdir $$n; mkdir $$n)
	$(XSLTPROC) --stringparam mode header \
	            --stringparam base-path $(XCBPROTO_XCBINCLUDEDIR)/ \
	            --stringparam extension-path $(XCBPROTO_XCBINCLUDEDIR)/ \
	            -o $@ $(srcdir)/c-client.xsl $< 

.xml.c:
	@n=`dirname $*`; test -d $$n || (echo mkdir $$n; mkdir $$n)
	$(XSLTPROC) --stringparam mode source \
	            --stringparam base-path $(XCBPROTO_XCBINCLUDEDIR)/ \
	            --stringparam extension-path $(XCBPROTO_XCBINCLUDEDIR)/ \
	            -o $@ $(srcdir)/c-client.xsl $< 

$(EXTENSION_XML):
	$(LN_S) -f $(XCBPROTO_XCBINCLUDEDIR)/$@ $@
