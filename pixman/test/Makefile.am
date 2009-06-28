if HAVE_GTK
TESTPROGRAMS =			\
	clip-test		\
	composite-test		\
	gradient-test		\
	region-test		\
	fetch-test

noinst_PROGRAMS = $(TESTPROGRAMS)

INCLUDES = -I$(top_srcdir)/pixman -I$(top_builddir)/pixman $(GTK_CFLAGS)

composite_test_LDADD =	$(top_builddir)/pixman/libpixman-1.la $(GTK_LIBS)
gradient_test_LDADD = $(top_builddir)/pixman/libpixman-1.la $(GTK_LIBS)
fetch_test_LDADD = $(top_builddir)/pixman/libpixman-1.la
region_test_LDADD = $(top_builddir)/pixman/libpixman-1.la
clip_test_LDADD = $(top_builddir)/pixman/libpixman-1.la $(GTK_LIBS)

endif
