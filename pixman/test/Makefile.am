AM_CFLAGS = @OPENMP_CFLAGS@
AM_LDFLAGS = @OPENMP_CFLAGS@

TEST_LDADD = $(top_builddir)/pixman/libpixman-1.la -lm
INCLUDES = -I$(top_srcdir)/pixman -I$(top_builddir)/pixman

TESTPROGRAMS =			\
	a1-trap-test		\
	pdf-op-test		\
	region-test		\
	region-translate-test	\
	fetch-test		\
	oob-test		\
	trap-crasher		\
	alpha-loop		\
	scaling-crash-test	\
	gradient-crash-test	\
	alphamap		\
	stress-test		\
	blitters-test		\
	scaling-test		\
	affine-test		\
	composite

a1_trap_test_LDADD = $(TEST_LDADD)
a1_trap_test_LDFLAGS = @TESTPROGS_EXTRA_LDFLAGS@

fetch_test_LDADD = $(TEST_LDADD)
fetch_test_LDFLAGS = @TESTPROGS_EXTRA_LDFLAGS@

trap_crasher_LDADD = $(TEST_LDADD)
trap_crasher_LDFLAGS = @TESTPROGS_EXTRA_LDFLAGS@

oob_test_LDADD = $(TEST_LDADD)
oob_test_LDFLAGS = @TESTPROGS_EXTRA_LDFLAGS@

scaling_crash_test_LDADD = $(TEST_LDADD)
scaling_crash_test_LDFLAGS = @TESTPROGS_EXTRA_LDFLAGS@

region_translate_test_LDADD = $(TEST_LDADD)
region_translate_test_LDFLAGS = @TESTPROGS_EXTRA_LDFLAGS@

pdf_op_test_LDADD = $(TEST_LDADD)
pdf_op_test_LDFLAGS = @TESTPROGS_EXTRA_LDFLAGS@
pdf_op_test_SOURCES = pdf-op-test.c utils.c utils.h

region_test_LDADD = $(TEST_LDADD)
region_test_LDFLAGS = @TESTPROGS_EXTRA_LDFLAGS@
region_test_SOURCES = region-test.c utils.c utils.h

blitters_test_LDADD = $(TEST_LDADD)
blitters_test_LDFLAGS = @TESTPROGS_EXTRA_LDFLAGS@
blitters_test_SOURCES = blitters-test.c utils.c utils.h

scaling_test_LDADD = $(TEST_LDADD)
scaling_test_LDFLAGS = @TESTPROGS_EXTRA_LDFLAGS@
scaling_test_SOURCES = scaling-test.c utils.c utils.h

affine_test_LDADD = $(TEST_LDADD)
affine_test_LDFLAGS = @TESTPROGS_EXTRA_LDFLAGS@
affine_test_SOURCES = affine-test.c utils.c utils.h

alphamap_LDADD = $(TEST_LDADD)
alphamap_LDFLAGS = @TESTPROGS_EXTRA_LDFLAGS@
alphamap_SOURCES = alphamap.c utils.c utils.h

alpha_loop_LDADD = $(TEST_LDADD)
alpha_loop_LDFLAGS = @TESTPROGS_EXTRA_LDFLAGS@
alpha_loop_SOURCES = alpha-loop.c utils.c utils.h

composite_LDADD = $(TEST_LDADD)
composite_LDFLAGS = @TESTPROGS_EXTRA_LDFLAGS@
composite_SOURCES = composite.c utils.c utils.h

gradient_crash_test_LDADD = $(TEST_LDADD)
gradient_crash_test_LDFLAGS = @TESTPROGS_EXTRA_LDFLAGS@
gradient_crash_test_SOURCES = gradient-crash-test.c utils.c utils.h

stress_test_LDADD = $(TEST_LDADD)
stress_test_LDFLAGS = @TESTPROGS_EXTRA_LDFLAGS@
stress_test_SOURCES = stress-test.c utils.c utils.h

# GTK using test programs

if HAVE_GTK

GTK_LDADD = $(TEST_LDADD) $(GTK_LIBS)
GTK_UTILS = gtk-utils.c gtk-utils.h

TESTPROGRAMS_GTK =		\
	clip-test		\
	clip-in			\
	composite-test		\
	gradient-test		\
	radial-test		\
	alpha-test		\
	screen-test		\
	convolution-test	\
	trap-test

INCLUDES += $(GTK_CFLAGS)

gradient_test_LDADD = $(GTK_LDADD)
gradient_test_SOURCES = gradient-test.c $(GTK_UTILS)

radial_test_LDADD = $(GTK_LDADD)
radial_test_SOURCES = radial-test.c utils.c utils.h $(GTK_UTILS)

alpha_test_LDADD = $(GTK_LDADD)
alpha_test_SOURCES = alpha-test.c $(GTK_UTILS)

composite_test_LDADD = $(GTK_LDADD)
composite_test_SOURCES = composite-test.c $(GTK_UTILS)

clip_test_LDADD = $(GTK_LDADD)
clip_test_SOURCES = clip-test.c $(GTK_UTILS)

clip_in_LDADD = $(GTK_LDADD)
clip_in_SOURCES = clip-in.c $(GTK_UTILS)

trap_test_LDADD = $(GTK_LDADD)
trap_test_SOURCES = trap-test.c $(GTK_UTILS)

screen_test_LDADD = $(GTK_LDADD)
screen_test_SOURCES = screen-test.c $(GTK_UTILS)

convolution_test_LDADD = $(GTK_LDADD)
convolution_test_SOURCES = convolution-test.c $(GTK_UTILS)

endif

# Benchmarks

BENCHMARKS =			\
	lowlevel-blt-bench

lowlevel_blt_bench_SOURCES = lowlevel-blt-bench.c utils.c utils.h
lowlevel_blt_bench_LDADD = $(TEST_LDADD)

noinst_PROGRAMS = $(TESTPROGRAMS) $(TESTPROGRAMS_GTK) $(BENCHMARKS)

TESTS = $(TESTPROGRAMS)
