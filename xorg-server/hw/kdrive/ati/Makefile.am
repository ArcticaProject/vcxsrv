#if DRI
#DRI_INCLUDES = 	-I$(top_srcdir)/dri	\
#		-I$(top_srcdir)/drm
#DRI_LIBS = 	$(top_builddir)/dri/libdri.a	\
#		$(top_builddir)/drm/libdrm.a
#DRI_SOURCES =	ati_dri.c	\
#		ati_dri.h	\
#		ati_dripriv.h	\
#		r128_common.h	\
#		r128_sarea.h	\
#		radeon_common.h	\
#		radeon_sarea.h
#endif

if KDRIVEFBDEV
FBDEV_INCLUDES =-I$(top_srcdir)/hw/kdrive/fbdev
FBDEV_LIBS = $(top_builddir)/hw/kdrive/fbdev/libfbdev.a
endif

if KDRIVEVESA
VESA_INCLUDES = -I$(top_srcdir)/hw/kdrive/vesa
VESA_LIBS = $(top_builddir)/hw/kdrive/vesa/libvesa.a
endif

INCLUDES = 					\
	@KDRIVE_INCS@				\
	$(DRI_INCLUDES)				\
	$(FBDEV_INCLUDES)			\
	$(VESA_INCLUDES)			\
	@KDRIVE_CFLAGS@

bin_PROGRAMS = Xati

noinst_LIBRARIES = libati.a

libati_a_SOURCES = 	\
	ati_cursor.c	\
	ati_dma.c	\
	ati_dma.h	\
	ati_draw.c	\
	ati_draw.h	\
	ati_microcode.c	\
	ati.c		\
	ati.h		\
	ati_reg.h	\
	r128_composite.c \
	ati_video.c	\
	radeon_composite.c \
	$(DRI_SOURCES)

Xati_SOURCES = \
	ati_stub.c

ATI_LIBS =				\
	libati.a 			\
	$(FBDEV_LIBS)			\
	$(VESA_LIBS)			\
	$(DRI_LIBS)			\
	@KDRIVE_LIBS@

if GLX
Xati_LDFLAGS = $(LD_EXPORT_SYMBOLS_FLAG)
endif

Xati_LDADD = \
	$(ATI_LIBS)			\
	@KDRIVE_LIBS@

Xati_DEPENDENCIES =	\
	libati.a 			\
	@KDRIVE_LOCAL_LIBS@

relink:
	rm -f $(bin_PROGRAMS) && make $(bin_PROGRAMS)
