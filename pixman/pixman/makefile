LIBRARY = libpixman-1

INCLUDES += $(OBJDIR)

CSRCS =			\
	pixman-access.c				\
	pixman-access-accessors.c		\
	pixman-cpu.c				\
	pixman-gradient-walker.c		\
	pixman-region16.c			\
	pixman-region32.c			\
	pixman-image.c				\
	pixman-implementation.c			\
	pixman-combine32.c			\
	pixman-combine64.c			\
	pixman-general.c			\
	pixman.c				\
	pixman-fast-path.c			\
	pixman-solid-fill.c			\
	pixman-conical-gradient.c		\
	pixman-linear-gradient.c		\
	pixman-radial-gradient.c		\
	pixman-bits-image.c			\
	pixman-utils.c				\
	pixman-edge.c				\
	pixman-edge-accessors.c			\
	pixman-trap.c				\
	pixman-timer.c				\
	pixman-matrix.c

BUILT_SOURCES = pixman-combine32.h pixman-combine32.c pixman-combine64.h pixman-combine64.c

$(OBJDIR)\pixman-combine32.c : pixman-combine.c.template pixman-combine32.h make-combine.pl
	perl make-combine.pl 8 < pixman-combine.c.template > $@
$(OBJDIR)\pixman-combine32.h : pixman-combine.h.template make-combine.pl
	perl make-combine.pl 8 < pixman-combine.h.template > $@

$(OBJDIR)\pixman-combine64.c : pixman-combine.c.template pixman-combine64.h make-combine.pl
	perl make-combine.pl 16 < pixman-combine.c.template > $@
$(OBJDIR)\pixman-combine64.h : pixman-combine.h.template make-combine.pl
	perl make-combine.pl 16 < pixman-combine.h.template > $@

## mmx code
#if USE_MMX
#noinst_LTLIBRARIES += libpixman-mmx.la
CSRCS += \
	pixman-mmx.c
#	pixman-mmx.h
#libpixman_mmx_la_CFLAGS = $(DEP_CFLAGS) $(MMX_CFLAGS)
#libpixman_mmx_la_LIBADD = $(DEP_LIBS)
#libpixman_1_la_LIBADD += libpixman-mmx.la
#endif

## vmx code
#if USE_VMX
#noinst_LTLIBRARIES += libpixman-vmx.la
#libpixman_vmx_la_SOURCES = \
#	pixman-vmx.c \
#	pixman-vmx.h \
#	pixman-combine32.h
#libpixman_vmx_la_CFLAGS = $(DEP_CFLAGS) $(VMX_CFLAGS)
#libpixman_vmx_la_LIBADD = $(DEP_LIBS)
#libpixman_1_la_LIBADD += libpixman-vmx.la
#endif

# sse2 code
#if USE_SSE2
#noinst_LTLIBRARIES += libpixman-sse2.la
#libpixman_sse2_la_SOURCES = \
#	pixman-sse2.c \
#	pixman-sse2.h
#libpixman_sse2_la_CFLAGS = $(DEP_CFLAGS) $(SSE2_CFLAGS)
#libpixman_sse2_la_LIBADD = $(DEP_LIBS)
#libpixman_1_la_LIBADD += libpixman-sse2.la
#endif

