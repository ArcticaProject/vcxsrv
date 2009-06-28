SUBDIRS = module

AM_CFLAGS =		 \
	 $(DIX_CFLAGS) \
	-DHAVE_XGL_CONFIG_H \
	-DHAVE_DIX_CONFIG_H \
	$(XGLMODULES_CFLAGS) \
	-I$(top_srcdir)/glx \
	-I$(top_srcdir)/GL/include \
	-I$(top_srcdir)/hw/xgl \
	-I@MESA_SOURCE@/include \
	-I@MESA_SOURCE@/src/mesa/glapi

libxglglxext_libraries = libxglglxext.la
libxglglxext_la_SOURCES = \
	xglglxext.h	  \
	xglglxext.c       \
	xglglxlog.c

noinst_LTLIBRARIES = $(libxglglxext_libraries)

