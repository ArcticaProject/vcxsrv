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

bin_PROGRAMS = Xsis

noinst_LIBRARIES = libsis.a

libsis_a_SOURCES = 	\
	sis.c		\
	sis.h		\
	sis_draw.c	\
	sis_reg.h

Xsis_SOURCES = \
	sis_stub.c

SIS_LIBS =				\
	libsis.a 			\
	$(FBDEV_LIBS)			\
	$(VESA_LIBS)			\
	@KDRIVE_LIBS@

if GLX
Xsis_LDFLAGS = $(LD_EXPORT_SYMBOLS_FLAG)
endif

Xsis_LDADD = \
	$(SIS_LIBS)				\
	@KDRIVE_LIBS@                          \
	$(TSLIB_FLAG)

Xsis_DEPENDENCIES =	\
	libsis.a 			\
	@KDRIVE_LOCAL_LIBS@

relink:
	rm -f $(bin_PROGRAMS) && make $(bin_PROGRAMS)
