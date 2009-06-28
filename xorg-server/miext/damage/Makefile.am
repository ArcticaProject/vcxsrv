noinst_LTLIBRARIES = libdamage.la

AM_CFLAGS = $(DIX_CFLAGS)

INCLUDES = -I$(srcdir)/../cw -I$(top_srcdir)/hw/xfree86/os-support

if XORG
sdk_HEADERS = damage.h damagestr.h
endif

libdamage_la_SOURCES =	\
	damage.c	\
	damage.h	\
	damagestr.h
