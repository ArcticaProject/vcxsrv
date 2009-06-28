noinst_LTLIBRARIES = libcfb.la

include Makefile.am.inc

DISTCLEANFILES += cfbglrop8.c

libcfb_la_SOURCES = cfb8bit.c cfbteblt8.c cfbglrop8.c cfbpush8.c cfbrctstp8.c \
                    $(libcfb_gen_sources) $(libcfb_common_sources)

libcfb_la_LIBADD = ../mfb/libmfb.la

AM_CFLAGS = -DPSZ=8 $(DIX_CFLAGS) $(PLATFORMDEFS)

INCLUDES = $(CFB_INCLUDES) -I$(top_srcdir)/hw/xfree86/os-support  -I$(top_srcdir)/hw/xfree86/common

EXTRA_DIST = cfbline.c cfbfillarc.c cfbzerarc.c cfbblt.c cfbsolid.c \
             cfbtileodd.c cfbtile32.c cfb8line.c cfbply1rct.c cfbglblt8.c \
	     cfb16.h cfb24.h cfb32.h cfb8bit.h cfbrrop.h \
	     stip68kgnu.h stipmips.s stipsparc.s stipsprc32.s

sdk_HEADERS = cfb.h cfb32.h cfb16.h cfbmap.h cfbunmap.h cfbmskbits.h
