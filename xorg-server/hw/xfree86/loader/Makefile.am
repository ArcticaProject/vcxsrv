noinst_LIBRARIES = libloader.a

INCLUDES = $(XORG_INCS) -I$(srcdir)/../parser -I$(srcdir)/../dixmods/extmod \
	   -I$(srcdir)/../vbe -I$(top_srcdir)/miext/cw -I$(srcdir)/../int10 \
	   -I$(srcdir)/../ddc -I$(srcdir)/../i2c -I$(srcdir)/../modes \
	   -I$(srcdir)/../ramdac

#AM_LDFLAGS = -r
AM_CFLAGS = -DIN_LOADER $(DIX_CFLAGS) $(XORG_CFLAGS)

EXTRA_DIST = \
	dlloader.h \
	loader.h \
	loaderProcs.h

libloader_a_SOURCES = \
	loader.c \
	loaderProcs.h \
	loadext.c \
        loadmod.c \
        dlloader.c \
	os.c \
	dixsym.c \
	extsym.c \
	misym.c \
	xf86sym.c \
	sym.h
