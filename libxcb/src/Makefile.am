lib_LTLIBRARIES = libxcb.la

EXTHEADERS =	xproto.h \
		bigreq.h \
		xc_misc.h
EXTSOURCES =	xproto.c \
		bigreq.c \
		xc_misc.c
EXTENSION_XML = xproto.xml \
		bigreq.xml \
		xc_misc.xml

AM_CFLAGS = $(CWARNFLAGS) $(NEEDED_CFLAGS) $(XDMCP_CFLAGS)
libxcb_la_LIBADD = $(NEEDED_LIBS) $(XDMCP_LIBS)
libxcb_la_SOURCES = \
		xcb_conn.c xcb_out.c xcb_in.c xcb_ext.c xcb_xid.c \
		xcb_list.c xcb_util.c xcb_auth.c c_client.py
nodist_libxcb_la_SOURCES = xproto.c bigreq.c xc_misc.c

# Explanation for -version-info:
# -version-info current:revision:age
# The library supports interface (current-age) to interface current.
# * If you changed the source code at all, increment revision.
# * If you add an interface, increment current and age and set revision to 0.
# * If you change or remove an interface, increment current and set revision
#   and age to 0.
libxcb_la_LDFLAGS = -version-info 2:0:1

XCB_LIBS = libxcb.la

# FIXME: find a way to autogenerate this from the XML files.

EXTHEADERS += composite.h
EXTSOURCES += composite.c
EXTENSION_XML += composite.xml
if BUILD_COMPOSITE
lib_LTLIBRARIES += libxcb-composite.la
libxcb_composite_la_LDFLAGS = -version-info 0:0:0
libxcb_composite_la_LIBADD = $(XCB_LIBS)
nodist_libxcb_composite_la_SOURCES = composite.c composite.h
endif

EXTHEADERS += damage.h
EXTSOURCES += damage.c
EXTENSION_XML += damage.xml
if BUILD_DAMAGE
lib_LTLIBRARIES += libxcb-damage.la
libxcb_damage_la_LDFLAGS = -version-info 0:0:0
libxcb_damage_la_LIBADD = $(XCB_LIBS)
nodist_libxcb_damage_la_SOURCES = damage.c damage.h
endif

EXTHEADERS += dpms.h
EXTSOURCES += dpms.c
EXTENSION_XML += dpms.xml
if BUILD_DPMS
lib_LTLIBRARIES += libxcb-dpms.la
libxcb_dpms_la_LDFLAGS = -version-info 0:0:0
libxcb_dpms_la_LIBADD = $(XCB_LIBS)
nodist_libxcb_dpms_la_SOURCES = dpms.c dpms.h
endif

EXTHEADERS += glx.h
EXTSOURCES += glx.c
EXTENSION_XML += glx.xml
if BUILD_GLX
lib_LTLIBRARIES += libxcb-glx.la
libxcb_glx_la_LDFLAGS = -version-info 0:0:0
libxcb_glx_la_LIBADD = $(XCB_LIBS)
nodist_libxcb_glx_la_SOURCES = glx.c glx.h
endif

EXTHEADERS += randr.h
EXTSOURCES += randr.c
EXTENSION_XML += randr.xml
if BUILD_RANDR
lib_LTLIBRARIES += libxcb-randr.la
libxcb_randr_la_LDFLAGS = -version-info 1:0:1
libxcb_randr_la_LIBADD = $(XCB_LIBS)
nodist_libxcb_randr_la_SOURCES = randr.c randr.h
endif

EXTHEADERS += record.h
EXTSOURCES += record.c
EXTENSION_XML += record.xml
if BUILD_RECORD
lib_LTLIBRARIES += libxcb-record.la
libxcb_record_la_LDFLAGS = -version-info 0:0:0
libxcb_record_la_LIBADD = $(XCB_LIBS)
nodist_libxcb_record_la_SOURCES = record.c record.h
endif

EXTHEADERS += render.h
EXTSOURCES += render.c
EXTENSION_XML += render.xml
if BUILD_RENDER
lib_LTLIBRARIES += libxcb-render.la
libxcb_render_la_LDFLAGS = -version-info 0:0:0
libxcb_render_la_LIBADD = $(XCB_LIBS)
nodist_libxcb_render_la_SOURCES = render.c render.h
endif

EXTHEADERS += res.h
EXTSOURCES += res.c
EXTENSION_XML += res.xml
if BUILD_RESOURCE
lib_LTLIBRARIES += libxcb-res.la
libxcb_res_la_LDFLAGS = -version-info 0:0:0
libxcb_res_la_LIBADD = $(XCB_LIBS)
nodist_libxcb_res_la_SOURCES = res.c res.h
endif

EXTHEADERS += screensaver.h
EXTSOURCES += screensaver.c
EXTENSION_XML += screensaver.xml
if BUILD_SCREENSAVER
lib_LTLIBRARIES += libxcb-screensaver.la
libxcb_screensaver_la_LDFLAGS = -version-info 0:0:0
libxcb_screensaver_la_LIBADD = $(XCB_LIBS)
nodist_libxcb_screensaver_la_SOURCES = screensaver.c screensaver.h
endif

EXTHEADERS += shape.h
EXTSOURCES += shape.c
EXTENSION_XML += shape.xml
if BUILD_SHAPE
lib_LTLIBRARIES += libxcb-shape.la
libxcb_shape_la_LDFLAGS = -version-info 0:0:0
libxcb_shape_la_LIBADD = $(XCB_LIBS)
nodist_libxcb_shape_la_SOURCES = shape.c shape.h
endif

EXTHEADERS += shm.h
EXTSOURCES += shm.c
EXTENSION_XML += shm.xml
if BUILD_SHM
lib_LTLIBRARIES += libxcb-shm.la
libxcb_shm_la_LDFLAGS = -version-info 0:0:0
libxcb_shm_la_LIBADD = $(XCB_LIBS)
nodist_libxcb_shm_la_SOURCES = shm.c shm.h
endif

EXTHEADERS += sync.h
EXTSOURCES += sync.c
EXTENSION_XML += sync.xml
if BUILD_SYNC
lib_LTLIBRARIES += libxcb-sync.la
libxcb_sync_la_LDFLAGS = -version-info 0:0:0
libxcb_sync_la_LIBADD = $(XCB_LIBS)
nodist_libxcb_sync_la_SOURCES = sync.c sync.h
endif

EXTHEADERS += xevie.h
EXTSOURCES += xevie.c
EXTENSION_XML += xevie.xml
if BUILD_XEVIE
lib_LTLIBRARIES += libxcb-xevie.la
libxcb_xevie_la_LDFLAGS = -version-info 0:0:0
libxcb_xevie_la_LIBADD = $(XCB_LIBS)
nodist_libxcb_xevie_la_SOURCES = xevie.c xevie.h
endif

EXTHEADERS += xf86dri.h
EXTSOURCES += xf86dri.c
EXTENSION_XML += xf86dri.xml
if BUILD_XFREE86_DRI
lib_LTLIBRARIES += libxcb-xf86dri.la
libxcb_xf86dri_la_LDFLAGS = -version-info 0:0:0
libxcb_xf86dri_la_LIBADD = $(XCB_LIBS)
nodist_libxcb_xf86dri_la_SOURCES = xf86dri.c xf86dri.h
endif

EXTHEADERS += xfixes.h
EXTSOURCES += xfixes.c
EXTENSION_XML += xfixes.xml
if BUILD_XFIXES
lib_LTLIBRARIES += libxcb-xfixes.la
libxcb_xfixes_la_LDFLAGS = -version-info 0:0:0
libxcb_xfixes_la_LIBADD = $(XCB_LIBS)
nodist_libxcb_xfixes_la_SOURCES = xfixes.c xfixes.h
endif

EXTHEADERS += xinerama.h
EXTSOURCES += xinerama.c
EXTENSION_XML += xinerama.xml
if BUILD_XINERAMA
lib_LTLIBRARIES += libxcb-xinerama.la
libxcb_xinerama_la_LDFLAGS = -version-info 0:0:0
libxcb_xinerama_la_LIBADD = $(XCB_LIBS)
nodist_libxcb_xinerama_la_SOURCES = xinerama.c xinerama.h
endif

EXTHEADERS += xinput.h
EXTSOURCES += xinput.c
EXTENSION_XML += xinput.xml
if BUILD_XINPUT
lib_LTLIBRARIES += libxcb-xinput.la
libxcb_xinput_la_LDFLAGS = -version-info 0:0:0
libxcb_xinput_la_LIBADD = $(XCB_LIBS)
nodist_libxcb_xinput_la_SOURCES = xinput.c xinput.h
endif

EXTHEADERS += xprint.h
EXTSOURCES += xprint.c
EXTENSION_XML += xprint.xml
if BUILD_XPRINT
lib_LTLIBRARIES += libxcb-xprint.la
libxcb_xprint_la_LDFLAGS = -version-info 0:0:0
libxcb_xprint_la_LIBADD = $(XCB_LIBS)
nodist_libxcb_xprint_la_SOURCES = xprint.c xprint.h
endif

EXTHEADERS += xselinux.h
EXTSOURCES += xselinux.c
EXTENSION_XML += xselinux.xml
if BUILD_SELINUX
lib_LTLIBRARIES += libxcb-xselinux.la
libxcb_xselinux_la_LDFLAGS = -version-info 0:0:0
libxcb_xselinux_la_LIBADD = $(XCB_LIBS)
nodist_libxcb_xselinux_la_SOURCES = xselinux.c xselinux.h
endif

EXTHEADERS += xtest.h
EXTSOURCES += xtest.c
EXTENSION_XML += xtest.xml
if BUILD_XTEST
lib_LTLIBRARIES += libxcb-xtest.la
libxcb_xtest_la_LDFLAGS = -version-info 0:0:0
libxcb_xtest_la_LIBADD = $(XCB_LIBS)
nodist_libxcb_xtest_la_SOURCES = xtest.c xtest.h
endif

EXTHEADERS += xv.h
EXTSOURCES += xv.c
EXTENSION_XML += xv.xml
if BUILD_XV
lib_LTLIBRARIES += libxcb-xv.la
libxcb_xv_la_LDFLAGS = -version-info 0:0:0
libxcb_xv_la_LIBADD = $(XCB_LIBS)
nodist_libxcb_xv_la_SOURCES = xv.c xv.h
endif

EXTHEADERS += xvmc.h
EXTSOURCES += xvmc.c
EXTENSION_XML += xvmc.xml
if BUILD_XVMC
lib_LTLIBRARIES += libxcb-xvmc.la
libxcb_xvmc_la_LDFLAGS = -version-info 0:0:0
libxcb_xvmc_la_LIBADD = $(XCB_LIBS)
nodist_libxcb_xvmc_la_SOURCES = xvmc.c xvmc.h
endif



xcbinclude_HEADERS = xcb.h xcbext.h
nodist_xcbinclude_HEADERS = $(EXTHEADERS)
noinst_HEADERS = xcbint.h

BUILT_SOURCES = $(EXTSOURCES) $(EXTHEADERS)
CLEANFILES = $(EXTSOURCES) $(EXTHEADERS) $(EXTENSION_XML)

$(EXTHEADERS) $(EXTSOURCES): c_client.py

SUFFIXES = .xml

.xml.h:
	$(PYTHON) $(srcdir)/c_client.py -p $(XCBPROTO_XCBPYTHONDIR) $(XCBPROTO_XCBINCLUDEDIR)/$<

.xml.c:
	$(PYTHON) $(srcdir)/c_client.py -p $(XCBPROTO_XCBPYTHONDIR) $(XCBPROTO_XCBINCLUDEDIR)/$<

$(EXTENSION_XML):
	$(LN_S) -f $(XCBPROTO_XCBINCLUDEDIR)/$@ $@
