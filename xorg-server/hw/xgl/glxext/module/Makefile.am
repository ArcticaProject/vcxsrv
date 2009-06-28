AM_CFLAGS = \
	$(DIX_CFLAGS) \
	-DHAVE_XGL_CONFIG_H \
	-DHAVE_DIX_CONFIG_H \
	$(XGLMODULES_CFLAGS) \
	-I$(top_srcdir)/hw/xgl 

libglx_la_LDFLAGS = -avoid-version
libglx_la_SOURCES = glxmodule.c
libglx_la_LIBADD  = $(top_builddir)/glx/libglx.la
libglx_modules    = libglx.la

libglcore_la_LDFLAGS = -avoid-version
libglcore_la_SOURCES = glcoremodule.c
libglcore_la_LIBADD  = $(top_builddir)/GL/mesa/libGLcore.la
libglcore_modules    = libglcore.la

moduledir = @XGL_MODULE_PATH@

module_LTLIBRARIES =	     \
	$(libglcore_modules) \
	$(libglx_modules)
