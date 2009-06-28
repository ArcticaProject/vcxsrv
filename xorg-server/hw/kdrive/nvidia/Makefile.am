INCLUDES = 					\
	@KDRIVE_INCS@				\
	-I$(top_srcdir)/hw/kdrive/vesa		\
	@KDRIVE_CFLAGS@

bin_PROGRAMS = Xnvidia

noinst_LIBRARIES = libnvidia.a

#	nvidiavideo.c

libnvidia_a_SOURCES = 	\
	nvidia.c	\
	nvidiadraw.c	\
	nvidia.h	\
	nvidiadraw.h

Xnvidia_SOURCES = \
	nvidiastub.c

NVIDIA_LIBS =					\
	libnvidia.a 				\
	$(top_builddir)/hw/kdrive/vesa/libvesa.a \
	@KDRIVE_LIBS@

if GLX
Xnvidia_LDFLAGS = $(LD_EXPORT_SYMBOLS_FLAG)
endif

Xnvidia_LDADD = \
	$(NVIDIA_LIBS)				\
	@KDRIVE_LIBS@

Xnvidia_DEPENDENCIES =	\
	libnvidia.a 				\
	@KDRIVE_LOCAL_LIBS@

relink:
	rm -f $(bin_PROGRAMS) && make $(bin_PROGRAMS)
