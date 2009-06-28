INCLUDES = 					\
	-I$(top_srcdir)/hw/kdrive/fbdev		\
	-I$(top_srcdir)/hw/kdrive/vesa		\
	@KDRIVE_INCS@				\
	@KDRIVE_CFLAGS@

bin_PROGRAMS = Xsmi

noinst_LIBRARIES = libsmi.a

#	smivideo.c	# not ready yet

libsmi_a_SOURCES = 	\
	smi.c		\
	smi.h		\
	smidraw.c	\
	smidraw.h

Xsmi_SOURCES = \
	smistub.c

SMI_LIBS =					\
	libsmi.a 				\
	$(top_builddir)/hw/kdrive/fbdev/libfbdev.a \
	$(top_builddir)/hw/kdrive/vesa/libvesa.a \
	@KDRIVE_LIBS@

if GLX
Xsmi_LDFLAGS = $(LD_EXPORT_SYMBOLS_FLAG)
endif

Xsmi_LDADD = \
	$(SMI_LIBS)				\
	@KDRIVE_LIBS@

Xsmi_DEPENDENCIES =	\
	libsmi.a 				\
	@KDRIVE_LOCAL_LIBS@

relink:
	rm -f $(bin_PROGRAMS) && make $(bin_PROGRAMS)
