noinst_LIBRARIES = libloader.a

INCLUDES = $(XORG_INCS) -I$(srcdir)/../parser -I$(srcdir)/../dixmods/extmod \
	   -I$(srcdir)/../vbe -I$(top_srcdir)/miext/cw -I$(srcdir)/../int10 \
	   -I$(srcdir)/../ddc -I$(srcdir)/../i2c -I$(srcdir)/../modes \
	   -I$(srcdir)/../ramdac

#AM_LDFLAGS = -r
AM_CFLAGS = -DIN_LOADER $(DIX_CFLAGS) $(XORG_CFLAGS)

if XORG_LOADER_SPARC
SPARC_SOURCES = SparcMulDiv.S
endif

EXTRA_DIST = \
	dlloader.h \
	loader.h \
	loaderProcs.h \
	SparcMulDiv.S

libloader_a_SOURCES = \
	loader.c \
	loaderProcs.h \
	loadext.c \
        loadmod.c \
	loadfont.c \
        dlloader.c \
	os.c \
	dixsym.c \
	extsym.c \
	fontsym.c \
	misym.c \
	xf86sym.c \
	sym.h \
	$(SPARC_SOURCES)
