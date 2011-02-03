LIBRARY=libwinglx

INCLUDES += $(OBJDIR) ..\..\..\glx .. ..\..\..
DEFINES += GLAPIENTRY=__stdcall _GLAPI_NO_EXPORTS

DEFINES += XWIN_CLIPBOARD
DEFINES += XWIN_MULTIWINDOW
DEFINES += XWIN_RANDR
DEFINES += RELOCATE_PROJECTROOT
INCLUDES += ..\..\..\miext\rootless
DEFINES += HAVE_XWIN_CONFIG_H PTW32_STATIC_LIB

CSRCS = \
	winpriv.c \
	glwrap.c \
	indirect.c \
	wgl_ext_api.c

.PHONY: getspecfiles

getspecfiles:
	wget http://www.opengl.org/registry/api/gl.spec
	wget http://www.opengl.org/registry/api/gl.tm

	wget http://www.opengl.org/registry/api/glxenum.spec
	wget http://www.opengl.org/registry/api/glxenumext.spec
	wget http://www.opengl.org/registry/api/glx.spec
	wget http://www.opengl.org/registry/api/glxext.spec
	wget http://www.opengl.org/registry/api/glx.tm

	wget http://www.opengl.org/registry/api/wglenum.spec
	wget http://www.opengl.org/registry/api/wglenumext.spec
	wget http://www.opengl.org/registry/api/wgl.spec
	wget http://www.opengl.org/registry/api/wglext.spec
	wget http://www.opengl.org/registry/api/wgl.tm
	wget http://www.opengl.org/registry/api/wglext.h
	wget http://www.opengl.org/registry/api/glext.h
	wget http://www.opengl.org/registry/api/glxext.h

$(OBJDIR)\generated_gl_wrappers.c: gen_gl_wrappers.py gl.spec gl.tm
	gen_gl_wrappers --spec=gl.spec --typemap=gl.tm --dispatch-header=../../../glx/dispatch.h --staticwrappers > $@

$(OBJDIR)\generated_wgl_wrappers.c: gen_gl_wrappers.py wglext.spec wgl.tm
	gen_gl_wrappers --spec=wglext.spec --typemap=wgl.tm --prefix=wgl --preresolve > $@

