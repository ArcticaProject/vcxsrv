XCBPROTO_XCBPYTHONDIR = ..\xcb-proto
XCBPROTO_XCBINCLUDEDIR = ..\xcb-proto\src

LIBRARY=libxcb

CSRCS = \
		xcb_conn.c xcb_out.c xcb_in.c xcb_ext.c xcb_xid.c \
		xcb_list.c xcb_util.c xcb_auth.c \
		xproto.c bigreq.c xc_misc.c

# FIXME: find a way to autogenerate this from the XML files.

EXTHEADERS += composite.h
EXTSOURCES += composite.c
EXTENSION_XML += composite.xml

EXTHEADERS += damage.h
EXTSOURCES += damage.c
EXTENSION_XML += damage.xml

EXTHEADERS += dpms.h
EXTSOURCES += dpms.c
EXTENSION_XML += dpms.xml

EXTHEADERS += glx.h
EXTSOURCES += glx.c
EXTENSION_XML += glx.xml

EXTHEADERS += randr.h
EXTSOURCES += randr.c
EXTENSION_XML += randr.xml

EXTHEADERS += record.h
EXTSOURCES += record.c
EXTENSION_XML += record.xml

EXTHEADERS += render.h
EXTSOURCES += render.c
EXTENSION_XML += render.xml

EXTHEADERS += res.h
EXTSOURCES += res.c
EXTENSION_XML += res.xml

EXTHEADERS += screensaver.h
EXTSOURCES += screensaver.c
EXTENSION_XML += screensaver.xml

EXTHEADERS += shape.h
EXTSOURCES += shape.c
EXTENSION_XML += shape.xml

EXTHEADERS += shm.h
EXTSOURCES += shm.c
EXTENSION_XML += shm.xml

EXTHEADERS += sync.h
EXTSOURCES += sync.c
EXTENSION_XML += sync.xml

EXTHEADERS += xevie.h
EXTSOURCES += xevie.c
EXTENSION_XML += xevie.xml

EXTHEADERS += xf86dri.h
EXTSOURCES += xf86dri.c
EXTENSION_XML += xf86dri.xml

EXTHEADERS += xfixes.h
EXTSOURCES += xfixes.c
EXTENSION_XML += xfixes.xml

EXTHEADERS += xinerama.h
EXTSOURCES += xinerama.c
EXTENSION_XML += xinerama.xml

EXTHEADERS += xinput.h
EXTSOURCES += xinput.c
EXTENSION_XML += xinput.xml

EXTHEADERS += xprint.h
EXTSOURCES += xprint.c
EXTENSION_XML += xprint.xml

EXTHEADERS += xselinux.h
EXTSOURCES += xselinux.c
EXTENSION_XML += xselinux.xml

EXTHEADERS += xtest.h
EXTSOURCES += xtest.c
EXTENSION_XML += xtest.xml

EXTHEADERS += xv.h
EXTSOURCES += xv.c
EXTENSION_XML += xv.xml

EXTHEADERS += xvmc.h
EXTSOURCES += xvmc.c
EXTENSION_XML += xvmc.xml

DEFINES += PTW32_STATIC_LIB

BUILT_SOURCES = $(EXTSOURCES) $(EXTHEADERS)

CSRCS += $(EXTSOURCES)

$(EXTHEADERS) $(EXTSOURCES): c_client.py

%.h: $(XCBPROTO_XCBINCLUDEDIR)\%.xml
	python c_client.py -p $(XCBPROTO_XCBPYTHONDIR) $<

%.c: $(XCBPROTO_XCBINCLUDEDIR)\%.xml
	python c_client.py -p $(XCBPROTO_XCBPYTHONDIR) $<

