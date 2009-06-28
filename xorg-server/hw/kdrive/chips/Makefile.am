INCLUDES = 					\
	@KDRIVE_INCS@				\
	-I$(top_srcdir)/hw/kdrive/vesa		\
	@KDRIVE_CFLAGS@

bin_PROGRAMS = Xchips

noinst_LIBRARIES = libchips.a

libchips_a_SOURCES = 	\
	chipsdraw.c	\
	chips.c		\
	chips.h	

Xchips_SOURCES = \
	chipsstub.c

CHIPS_LIBS =					 \
	libchips.a 				 \
	$(top_builddir)/hw/kdrive/vesa/libvesa.a \
	@KDRIVE_LIBS@

if GLX
Xchips_LDFLAGS = $(LD_EXPORT_SYMBOLS_FLAG)
endif

Xchips_LDADD = \
	$(CHIPS_LIBS)				\
	@KDRIVE_LIBS@

Xchips_DEPENDENCIES = 	\
	libchips.a 				 \
	@KDRIVE_LOCAL_LIBS@

relink:
	rm -f $(bin_PROGRAMS) && make $(bin_PROGRAMS)
