lib_LTLIBRARIES = libpixman-1.la
libpixman_1_la_LDFLAGS = -version-info $(LT_VERSION_INFO) -no-undefined
libpixman_1_la_LIBADD = @DEP_LIBS@ -lm
libpixman_1_la_CFLAGS = -DPIXMAN_DISABLE_DEPRECATED
libpixman_1_la_SOURCES =		\
	pixman.h			\
	pixman-access.c			\
	pixman-access-accessors.c	\
	pixman-region16.c		\
	pixman-region32.c		\
	pixman-private.h		\
	pixman-image.c			\
	pixman-combine32.c		\
	pixman-combine32.h		\
	pixman-combine64.c		\
	pixman-combine64.h		\
	pixman-compose.c		\
	pixman-compose-accessors.c	\
	pixman-pict.c			\
	pixman-source.c			\
	pixman-transformed.c		\
	pixman-transformed-accessors.c	\
	pixman-utils.c			\
	pixman-edge.c			\
	pixman-edge-accessors.c		\
	pixman-edge-imp.h		\
	pixman-trap.c			\
	pixman-compute-region.c		\
	pixman-timer.c

libpixmanincludedir = $(includedir)/pixman-1/
libpixmaninclude_HEADERS = pixman.h pixman-version.h
noinst_LTLIBRARIES = 

pixman-combine32.c : combine.inc pixman-combine32.h combine.pl
	$(PERL) $(srcdir)/combine.pl 8 < $(srcdir)/combine.inc > $@ || ($(RM) $@; exit 1)
pixman-combine32.h : combine.h.inc combine.pl
	$(PERL) $(srcdir)/combine.pl 8 < $(srcdir)/combine.h.inc > $@ || ($(RM) $@; exit 1)

pixman-combine64.c : combine.inc pixman-combine64.h combine.pl
	$(PERL) $(srcdir)/combine.pl 16 < $(srcdir)/combine.inc > $@ || ($(RM) $@; exit 1)
pixman-combine64.h : combine.h.inc combine.pl
	$(PERL) $(srcdir)/combine.pl 16 < $(srcdir)/combine.h.inc > $@ || ($(RM) $@; exit 1)

EXTRA_DIST = Makefile.win32 combine.inc combine.pl pixman-region.c combine.h.inc
CLEANFILES = pixman-combine32.c pixman-combine64.c pixman-combine32.h pixman-combine64.h

# mmx code
if USE_MMX
noinst_LTLIBRARIES += libpixman-mmx.la
libpixman_mmx_la_SOURCES = \
	pixman-mmx.c \
	pixman-mmx.h
libpixman_mmx_la_CFLAGS = $(DEP_CFLAGS) $(MMX_CFLAGS)
libpixman_mmx_la_LIBADD = $(DEP_LIBS)
libpixman_1_la_LIBADD += libpixman-mmx.la
endif

# vmx code
if USE_VMX
noinst_LTLIBRARIES += libpixman-vmx.la
libpixman_vmx_la_SOURCES = \
	pixman-vmx.c \
	pixman-vmx.h \
	pixman-combine32.h
libpixman_vmx_la_CFLAGS = $(DEP_CFLAGS) $(VMX_CFLAGS)
libpixman_vmx_la_LIBADD = $(DEP_LIBS)
libpixman_1_la_LIBADD += libpixman-vmx.la
endif

# sse2 code
if USE_SSE2
noinst_LTLIBRARIES += libpixman-sse2.la
libpixman_sse2_la_SOURCES = \
	pixman-sse2.c \
	pixman-sse2.h
libpixman_sse2_la_CFLAGS = $(DEP_CFLAGS) $(SSE2_CFLAGS)
libpixman_sse2_la_LIBADD = $(DEP_LIBS)
libpixman_1_la_LIBADD += libpixman-sse2.la
endif

