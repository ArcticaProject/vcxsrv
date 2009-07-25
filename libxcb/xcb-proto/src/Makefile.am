xcbinclude_HEADERS = \
	xcb.xsd \
	xproto.xml \
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
	xinput.xml \
	xprint.xml \
	xselinux.xml \
	xtest.xml \
	xv.xml \
	xvmc.xml

if HAVE_XMLLINT
check-local:
	$(XMLLINT) --noout --schema $(srcdir)/xcb.xsd $(srcdir)/*.xml
endif
