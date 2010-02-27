TEST_LDADD = $(top_builddir)/pixman/libpixman-1.la
INCLUDES = -I$(top_srcdir)/pixman -I$(top_builddir)/pixman

TESTPROGRAMS =			\
	a1-trap-test		\
	region-test		\
	fetch-test		\
	oob-test		\
	window-test		\
	trap-crasher		\
	alphamap		\
	blitters-test		\
	scaling-test		\
	composite

a1_trap_test_LDADD = $(TEST_LDADD)
fetch_test_LDADD = $(TEST_LDADD)
composite_LDADD = $(TEST_LDADD)
trap_crasher_LDADD = $(TEST_LDADD)
oob_test_LDADD = $(TEST_LDADD)
window_test_LDADD = $(TEST_LDADD)

region_test_LDADD = $(TEST_LDADD)
region_test_SOURCES = region-test.c utils.c utils.h

blitters_test_LDADD = $(TEST_LDADD)
blitters_test_SOURCES = blitters-test.c utils.c utils.h

scaling_test_LDADD = $(TEST_LDADD)
scaling_test_SOURCES = scaling-test.c utils.c utils.h

alphamap_LDADD = $(TEST_LDADD)
alphamap_SOURCES = alphamap.c utils.c utils.h

# GTK using test programs

if HAVE_GTK

GTK_LDADD = $(TEST_LDADD) $(GTK_LIBS)
GTK_UTILS = gtk-utils.c gtk-utils.h

TESTPROGRAMS_GTK =		\
	clip-test		\
	clip-in			\
	composite-test		\
	gradient-test		\
	alpha-test		\
	screen-test		\
	convolution-test	\
	trap-test		\
	alphamap

INCLUDES += $(GTK_CFLAGS)

gradient_test_LDADD = $(GTK_LDADD)
gradient_test_SOURCES = gradient-test.c $(GTK_UTILS)

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

noinst_PROGRAMS = $(TESTPROGRAMS) $(TESTPROGRAMS_GTK)

TESTS = $(TESTPROGRAMS)

