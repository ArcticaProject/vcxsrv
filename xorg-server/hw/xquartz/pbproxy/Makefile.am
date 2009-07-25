AM_CPPFLAGS=-F/System/Library/Frameworks/ApplicationServices.framework/Frameworks
AM_CFLAGS=$(XPBPROXY_CFLAGS)

noinst_LTLIBRARIES = libxpbproxy.la
libxpbproxy_la_SOURCES = \
	trick_autotools.c \
	main.m \
	x-input.m \
	x-selection.m

libxpbproxy_la_LDFLAGS=$(XPBPROXY_LIBS)

if STANDALONE_XPBPROXY

bin_PROGRAMS = xpbproxy
xpbproxy_SOURCES = app-main.m
xpbproxy_LDADD = libxpbproxy.la

endif

EXTRA_DIST = \
	pbproxy.h \
	x-selection.h
