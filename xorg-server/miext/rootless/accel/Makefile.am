AM_CFLAGS = $(DIX_CFLAGS) $(XSERVER_CFLAGS)
AM_CPPFLAGS = -I$(srcdir)/.. -I$(top_srcdir)/hw/xfree86/os-support

noinst_LTLIBRARIES = librlAccel.la
librlAccel_la_SOURCES = \
	rlAccel.c \
	rlBlt.c \
	rlCopy.c \
	rlFill.c \
	rlFillRect.c \
	rlFillSpans.c \
	rlGlyph.c \
	rlSolid.c

EXTRA_DIST = rlAccel.h
