INCLUDES = 					\
	@KDRIVE_INCS@				\
	@KDRIVE_CFLAGS@

noinst_LIBRARIES = libvesa.a

bin_PROGRAMS = Xvesa

libvesa_a_SOURCES = \
	vesa.c		\
	vesa.h		\
	vbe.c		\
	vbe.h		\
	vga.c		\
	vga.h		\
	vm86.c		\
	vm86.h

Xvesa_SOURCES = \
	vesainit.c

Xvesa_LDADD = \
	libvesa.a 				\
	@KDRIVE_LIBS@

Xvesa_DEPENDENCIES =	\
	libvesa.a 				\
	@KDRIVE_LOCAL_LIBS@

Xvesa_LDFLAGS = $(LD_EXPORT_SYMBOLS_FLAG)

relink:
	rm -f $(bin_PROGRAMS) && make $(bin_PROGRAMS)
