AM_CFLAGS = \
        $(DIX_CFLAGS) \
	-I$(srcdir)/.. \
	-I$(srcdir)/../.. \
	$(XEGLMODULES_CFLAGS)

libxegl_la_LDFLAGS = -avoid-version
libxegl_la_SOURCES = xeglmodule.c
libxegl_la_LIBADD   =	  \
	$(top_builddir)/hw/xgl/glx/libxegl.la \
	$(XEGLMODULES_LIBS)

moduledir = @xglmoduledir@

module_LTLIBRARIES = libxegl.la
