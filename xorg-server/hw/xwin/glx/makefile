LIBRARY=libwinglx

INCLUDES += ..\..\..\glx .. ..\..\..
DEFINES += GLAPIENTRY=__stdcall _GLAPI_NO_EXPORTS

DEFINES += XWIN_CLIPBOARD
DEFINES += XWIN_MULTIWINDOW
DEFINES +=  XWIN_NATIVEGDI
DEFINES += XWIN_RANDR RELOCATE_PROJECTROOT
INCLUDES += ..\..\..\miext\rootless
DEFINES += HAVE_XWIN_CONFIG_H BUILD_DATE=0-0-0000 PTW32_STATIC_LIB

CSRCS = \
	winpriv.c \
	glwrap.c \
	indirect.c \
	wgl_ext_api.c

glwrap.c: generated_gl_wrappers.c
wgl_ext_api.c: generated_wgl_wrappers.c wglext.h
wgl_ext_api.h: wglext.h
indirect.c: wgl_ext_api.h

SPEC_FILES = gl.spec gl.tm wglext.spec wgl.tm

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

generated_gl_wrappers.c: gen_gl_wrappers.py gl.spec gl.tm
	gen_gl_wrappers --spec=gl.spec --typemap=gl.tm --dispatch-header=../../../glx/dispatch.h --staticwrappers >generated_gl_wrappers.c

generated_wgl_wrappers.c: gen_gl_wrappers.py wglext.spec wgl.tm
	gen_gl_wrappers --spec=wglext.spec --typemap=wgl.tm --prefix=wgl --preresolve >generated_wgl_wrappers.c


BUILT_SOURCES = generated_gl_wrappers.c generated_wgl_wrappers.c

