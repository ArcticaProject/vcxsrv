noinst_LTLIBRARIES = libXwinGLX.la

libXwinGLX_la_SOURCES = \
	winpriv.c \
	winpriv.h \
	glwindows.h \
	glwrap.c \
	indirect.c \
	wgl_ext_api.c

if XWIN_MULTIWINDOW
DEFS_MULTIWINDOW = -DXWIN_MULTIWINDOW
endif

if XWIN_MULTIWINDOWEXTWM
DEFS_MULTIWINDOWEXTWM = -DXWIN_MULTIWINDOWEXTWM
endif

DEFS =  $(DEFS_MULTIWINDOW) $(DEFS_MULTIWINDOWEXTWM)

INCLUDES = -I$(top_srcdir)/miext/rootless

AM_CFLAGS = -DHAVE_XWIN_CONFIG_H $(DIX_CFLAGS) \
            $(XWINMODULES_CFLAGS) \
            -I$(top_srcdir) \
            -I$(top_srcdir)/hw/xwin/

glwrap.c: generated_gl_wrappers.c
wgl_ext_api.c: generated_wgl_wrappers.c wglext.h
wgl_ext_api.h: wglext.h
indirect.c: wgl_ext_api.h

SPEC_FILES = gl.spec gl.tm wglext.spec wgl.tm

gl.spec:
	wget http://www.opengl.org/registry/api/gl.spec

gl.tm:
	wget http://www.opengl.org/registry/api/gl.tm

wglext.spec:
	wget http://www.opengl.org/registry/api/wglext.spec

wgl.tm:
	wget http://www.opengl.org/registry/api/wgl.tm

generated_gl_wrappers.c: gen_gl_wrappers.py gl.spec gl.tm
	$(srcdir)/gen_gl_wrappers.py --spec=`test -e gl.spec || echo $(srcdir)/`gl.spec --typemap=`test -e gl.tm || echo $(srcdir)/`gl.tm --dispatch-header=$(top_srcdir)/glx/dispatch.h --staticwrappers >generated_gl_wrappers.c

generated_wgl_wrappers.c: gen_gl_wrappers.py wglext.spec wgl.tm
	$(srcdir)/gen_gl_wrappers.py --spec=`test -e wglext.spec || echo $(srcdir)/`wglext.spec --typemap=`test -e wgl.tm || echo $(srcdir)/`wgl.tm --prefix=wgl --preresolve >generated_wgl_wrappers.c

wglext.h:
	wget http://www.opengl.org/registry/api/wglext.h

BUILT_SOURCES = generated_gl_wrappers.c generated_wgl_wrappers.c
CLEANFILES = $(BUILT_SOURCES)
DISTCLEANFILES = $(SPEC_FILES) wglext.h

EXTRA_DIST = gen_gl_wrappers.py $(SPEC_FILES) wglext.h
