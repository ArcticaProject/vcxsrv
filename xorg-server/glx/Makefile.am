if AIGLX
GLXDRI_LIBRARY = libglxdri.la
endif

noinst_LTLIBRARIES = libglx.la $(GLXDRI_LIBRARY)

AM_CFLAGS = \
	@DIX_CFLAGS@ \
	@GL_CFLAGS@ \
	@XLIB_CFLAGS@ \
	@LIBDRM_CFLAGS@ \
	@DRIPROTO_CFLAGS@ \
	-DXFree86Server \
	@GLX_DEFINES@ \
	@GLX_ARCH_DEFINES@

# none yet
#sdk_HEADERS =

INCLUDES = \
	-I$(top_srcdir)/hw/xfree86/os-support \
	-I$(top_srcdir)/hw/xfree86/os-support/bus \
	-I$(top_srcdir)/hw/xfree86/common \
	-I$(top_srcdir)/hw/xfree86/dri \
	-I$(top_srcdir)/mi

if DRI2_AIGLX
INCLUDES += -I$(top_srcdir)/hw/xfree86/dri2
endif

glapi_sources =					\
	indirect_dispatch.c			\
	indirect_dispatch.h			\
	indirect_dispatch_swap.c		\
	indirect_reqsize.c			\
	indirect_reqsize.h			\
	indirect_size.h				\
	indirect_size_get.c			\
	indirect_size_get.h			\
	indirect_table.c			\
	dispatch.h				\
	glapitable.h				\
	glapitemp.h				\
	glapi.c					\
	glapi.h					\
	glapioffsets.h				\
	glprocs.h				\
	glthread.c				\
	glthread.h

libglxdri_la_SOURCES = \
        glxdri.c \
        extension_string.c \
        extension_string.h

if DRI2_AIGLX
libglxdri_la_SOURCES += glxdri2.c
endif

libglx_la_SOURCES = \
	$(indirect_sources) \
	$(glapi_sources) \
	indirect_util.c \
	indirect_util.h \
	indirect_program.c \
	indirect_table.h \
	indirect_texture_compression.c \
        g_disptab.h \
        glxbyteorder.h \
        glxcmds.c \
        glxcmdsswap.c \
        glxcontext.h \
        glxdrawable.h \
        glxext.c \
        glxext.h \
	glxdriswrast.c \
	glxdricommon.c \
	glxdricommon.h \
        glxscreens.c \
        glxscreens.h \
        glxserver.h \
        glxutil.h \
        render2.c \
        render2swap.c \
        renderpix.c \
        renderpixswap.c \
        rensize.c \
        single2.c \
        single2swap.c \
        singlepix.c \
        singlepixswap.c \
        singlesize.c \
        singlesize.h \
        swap_interval.c \
        unpack.h \
        xfont.c
