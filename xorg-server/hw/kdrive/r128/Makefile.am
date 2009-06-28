INCLUDES = 					\
	@KDRIVE_INCS@				\
	-I$(top_srcdir)/hw/kdrive/vesa		\
	@KDRIVE_CFLAGS@

bin_PROGRAMS = Xr128

noinst_LIBRARIES = libr128.a

libr128_a_SOURCES = 	\
	r128draw.c	\
	r128.c		\
	r128.h	

Xr128_SOURCES = \
	r128stub.c

R128_LIBS =					\
	libr128.a 				\
	$(top_builddir)/hw/kdrive/vesa/libvesa.a \
	@KDRIVE_LIBS@

if GLX
Xr128_LDFLAGS = $(LD_EXPORT_SYMBOLS_FLAG)
endif

Xr128_LDADD = \
	$(R128_LIBS)				\
	@KDRIVE_LIBS@

Xr128_DEPENDENCIES =	\
	libr128.a 				\
	@KDRIVE_LOCAL_LIBS@

relink:
	rm -f $(bin_PROGRAMS) && make $(bin_PROGRAMS)
