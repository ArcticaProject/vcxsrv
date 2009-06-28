INCLUDES = 					\
        @KDRIVE_INCS@				\
	-I$(top_srcdir)/hw/kdrive/vesa		\
	@KDRIVE_CFLAGS@

bin_PROGRAMS = Xvia

noinst_LIBRARIES = libvia.a

libvia_a_SOURCES = 	\
	via.c \
	viadraw.c \
	viadraw.h \
	via.h \
	via_regs.h

Xvia_SOURCES =	\
	viastub.c

VIA_LIBS =	\
	libvia.a	\
	$(top_builddir)/hw/kdrive/vesa/libvesa.a

if GLX
Xvia_LDFLAGS = $(LD_EXPORT_SYMBOLS_FLAG)
endif

Xvia_LDADD =						\
	$(VIA_LIBS)					\
	@KDRIVE_LIBS@

Xvia_DEPENDENCIES =	\
	libvia.a	\
	@KDRIVE_LOCAL_LIBS@

relink:
	rm -f $(bin_PROGRAMS) && make $(bin_PROGRAMS)
