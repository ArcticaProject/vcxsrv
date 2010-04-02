module_LTLIBRARIES = libexa.la

libexa_la_LDFLAGS = -avoid-version

INCLUDES = \
	$(XORG_INCS) \
	-I$(srcdir)/../../../exa \
	-I$(srcdir)/../../../miext/cw

AM_CFLAGS = $(DIX_CFLAGS) $(XORG_CFLAGS)

libexa_la_SOURCES = \
	examodule.c

libexa_la_LIBADD = \
	../../../exa/libexa.la

include $(top_srcdir)/cpprules.in

drivermandir = $(DRIVER_MAN_DIR)
driverman_DATA = exa.$(DRIVER_MAN_SUFFIX)
CLEANFILES = $(driverman_DATA) exa.man

exa.$(DRIVER_MAN_SUFFIX): exa.man
	-$(AM_V_at)rm -f exa.$(DRIVER_MAN_SUFFIX)
	$(AM_V_at)$(LN_S) exa.man exa.$(DRIVER_MAN_SUFFIX)

EXTRA_DIST = exa.man.pre
