INCLUDES = 					\
	@KDRIVE_INCS@				\
	-I$(top_srcdir)/hw/kdrive/vesa		\
	@KDRIVE_CFLAGS@

bin_PROGRAMS = Xmga

noinst_LIBRARIES = libmga.a

libmga_a_SOURCES = 		\
	mgadraw.c		\
	g400_composite.c	\
	g400_common.h		\
	mga.c			\
	mga.h	

Xmga_SOURCES = \
	mgastub.c

MGA_LIBS =						\
	libmga.a 					\
	$(top_builddir)/hw/kdrive/vesa/libvesa.a	\
	@KDRIVE_LIBS@

if GLX
Xmga_LDFLAGS = $(LD_EXPORT_SYMBOLS_FLAG)
endif

Xmga_LDADD = \
	$(MGA_LIBS)					\
	@KDRIVE_LIBS@

Xmga_DEPENDENCIES =	\
	libmga.a 					\
	@KDRIVE_LOCAL_LIBS@

relink:
	rm -f $(bin_PROGRAMS) && make $(bin_PROGRAMS)
