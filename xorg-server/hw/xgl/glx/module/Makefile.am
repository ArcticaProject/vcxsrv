if GLX
GLX_LIB = $(top_builddir)/hw/xgl/glxext/libxglglxext.la
endif

AM_CFLAGS = $(DIX_CFLAGS) \
	-I$(srcdir)/.. \
	-I$(srcdir)/../.. \
	$(XGLXMODULES_CFLAGS)

libxglx_la_DEPENDENCIES = $(GLX_LIB)
libxglx_la_LDFLAGS = -avoid-version
libxglx_la_SOURCES = xglxmodule.c
libxglx_la_LIBADD = \
	$(top_builddir)/hw/xgl/glx/libxglx.la \
	$(GLX_LIB) \
	$(XGLXMODULES_LIBS) \
	-lXrandr

moduledir = @xglmoduledir@

module_LTLIBRARIES = libxglx.la
