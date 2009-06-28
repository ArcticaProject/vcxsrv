noinst_LTLIBRARIES = libcfb32.la

include $(top_srcdir)/cfb/Makefile.am.inc

libcfb32_la_SOURCES = $(libcfb_common_sources) $(libcfb_gen_sources)

INCLUDES = $(CFB_INCLUDES) $(DIX_CFLAGS) -I$(top_srcdir)/hw/xfree86/os-support -I$(top_srcdir)/hw/xfree86/common

AM_CFLAGS = -DPSZ=32  $(PLATFORMDEFS)
