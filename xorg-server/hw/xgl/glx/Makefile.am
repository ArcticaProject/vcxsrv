if XGL
XGL_MODULE_DIRS = module
endif

DIST_SUBDIRS = module

SUBDIRS =		  \
	.		  \
	$(XGL_MODULE_DIRS)

AM_CFLAGS = \
	-I$(srcdir)/.. \
	-I$(srcdir)/../glxext \
	-I$(top_srcdir)/glx \
	-I$(top_srcdir)/GL/include \
	-I@MESA_SOURCE@/include \
	-I@MESA_SOURCE@/src/mesa/glapi \
	 $(DIX_CFLAGS) \
	-DHAVE_XGL_CONFIG_H \
	-DHAVE_DIX_CONFIG_H \
	$(XGLXMODULES_CFLAGS)

noinst_LTLIBRARIES = libxglx.la

libxglx_la_SOURCES = \
	xglx.h	     \
	xglx.c	     \
	xglxorg.c

# Xglx_DEPENDENCIES = @XGLX_LIBS@
# Xglx_LDFLAGS	  = -export-dynamic
# Xglx_SOURCES	  = \
#	xglxinit.c \
#	$(top_srcdir)/mi/miinitext.c \
#	$(top_srcdir)/Xext/dpmsstubs.c \
#	$(top_srcdir)/Xi/stubs.c \
#	$(top_srcdir)/fb/fbcmap.c
#Xglx_LDADD = \
#	libxglx.la \
#	../libxgl.a \
#	$(XORG_CORE_LIBS) \
#	$(XGLX_LIBS) \
#	$(EXTENSION_LIBS) \
#	$(XGLXMODULES_LIBS) \
#	-lXrandr
#Xglx_programs	  = Xglx
#
#bin_PROGRAMS = $(Xglx_programs)
