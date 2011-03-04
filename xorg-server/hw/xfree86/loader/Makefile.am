noinst_LTLIBRARIES = libloader.la

INCLUDES = $(XORG_INCS) -I$(srcdir)/../parser -I$(top_srcdir)/miext/cw \
	   -I$(srcdir)/../ddc -I$(srcdir)/../i2c -I$(srcdir)/../modes \
	   -I$(srcdir)/../ramdac

#AM_LDFLAGS = -r
AM_CFLAGS = $(DIX_CFLAGS) $(XORG_CFLAGS)

EXTRA_DIST = \
	loader.h \
	loaderProcs.h \
	sdksyms.sh

nodist_libloader_la_SOURCES = \
	sdksyms.c

libloader_la_SOURCES = \
	loader.c \
	loaderProcs.h \
	loadext.c \
        loadmod.c \
	os.c

libloader_la_LIBADD = $(DLOPEN_LIBS)

CLEANFILES = sdksyms.c sdksyms.dep

sdksyms.dep sdksyms.c: sdksyms.sh $(top_builddir)/include/do-not-use-config.h
	CPP='$(CPP)' AWK='$(AWK)' $(srcdir)/sdksyms.sh $(top_srcdir) $(AM_CFLAGS) $(CFLAGS) $(INCLUDES)

SDKSYMS_DEP = sdksyms.dep
include $(SDKSYMS_DEP)
