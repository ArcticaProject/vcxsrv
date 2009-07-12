CSRCS =	xproto.c \
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

LIBRARY=libxcb

CSRCS = \
		xcb_conn.c xcb_out.c xcb_in.c xcb_ext.c xcb_xid.c \
		xcb_list.c xcb_util.c xcb_auth.c xcb_xlib.c \
		xproto.c bigreq.c xc_misc.c
#                c-client.xsl \
#CSRCS =  xproto.c

DEFINES += PTW32_STATIC_LIB
#.xml.h:
#	@n=`dirname $*`; test -d $$n || (echo mkdir $$n; mkdir $$n)
#	$(XSLTPROC) --stringparam mode header \
#	            --stringparam base-path $(XCBPROTO_XCBINCLUDEDIR)/ \
#	            --stringparam extension-path $(XCBPROTO_XCBINCLUDEDIR)/ \
#	            -o $@ $(srcdir)/c-client.xsl $< 

#.xml.c:
#	@n=`dirname $*`; test -d $$n || (echo mkdir $$n; mkdir $$n)
#	$(XSLTPROC) --stringparam mode source \
#	            --stringparam base-path $(XCBPROTO_XCBINCLUDEDIR)/ \
#	            --stringparam extension-path $(XCBPROTO_XCBINCLUDEDIR)/ \
#	            -o $@ $(srcdir)/c-client.xsl $< 

#$(EXTENSION_XML):
#	$(LN_S) -f $(XCBPROTO_XCBINCLUDEDIR)/$@ $@

