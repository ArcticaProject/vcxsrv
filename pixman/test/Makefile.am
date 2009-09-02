TEST_LDADD = $(top_builddir)/pixman/libpixman-1.la
INCLUDES = -I$(top_srcdir)/pixman -I$(top_builddir)/pixman

TESTPROGRAMS =			\
	region-test		\
	scaling-test		\
	blitters-test		\
	fetch-test		\
	oob-test		\
	window-test		\
	trap-crasher

fetch_test_LDADD = $(TEST_LDADD)
region_test_LDADD = $(TEST_LDADD)
scaling_test_LDADD = $(TEST_LDADD)
blitters_test_LDADD = $(TEST_LDADD)
trap_crasher_LDADD = $(TEST_LDADD)
oob_test_LDADD = $(TEST_LDADD)
window_test_LDADD = $(TEST_LDADD)

# GTK using test programs

if HAVE_GTK

GTK_LDADD = $(TEST_LDADD) $(GTK_LIBS)

TESTPROGRAMS +=			\
	clip-test		\
	clip-in			\
	composite-test		\
	gradient-test		\
	alpha-test		\
	screen-test		\
	convolution-test	\
	trap-test

INCLUDES += $(GTK_CFLAGS)

gradient_test_LDADD = $(GTK_LDADD)
gradient_test_SOURCES = gradient-test.c utils.c utils.h

alpha_test_LDADD = $(GTK_LDADD)
alpha_test_SOURCES = alpha-test.c utils.c utils.h

composite_test_LDADD = $(GTK_LDADD)
composite_test_SOURCES = composite-test.c utils.c utils.h

clip_test_LDADD = $(GTK_LDADD)
clip_test_SOURCES = clip-test.c utils.c utils.h

clip_in_LDADD = $(GTK_LDADD)
clip_in_SOURCES = clip-in.c utils.c utils.h

trap_test_LDADD = $(GTK_LDADD)
trap_test_SOURCES = trap-test.c utils.c utils.h

screen_test_LDADD = $(GTK_LDADD)
screen_test_SOURCES = screen-test.c utils.c utils.h

convolution_test_LDADD = $(GTK_LDADD)
convolution_test_SOURCES = convolution-test.c utils.c utils.h

endif

noinst_PROGRAMS = $(TESTPROGRAMS)

