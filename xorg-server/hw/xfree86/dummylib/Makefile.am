# libdummy.a contains just those bits used in the server itself
# libdummy-nonserver.a contains additional routines normally found in the
# server for use in building the utilities like config tools

noinst_LIBRARIES = libdummy-nonserver.a

INCLUDES = $(XORG_INCS)

AM_CFLAGS = $(DIX_CFLAGS) $(XORG_CFLAGS)

if NEED_STRLCAT
STRL_SRCS = $(top_srcdir)/os/strlcat.c $(top_srcdir)/os/strlcpy.c
endif

libdummy_nonserver_a_SOURCES = \
	dixprivates.c \
	fatalerror.c \
	logvwrite.c \
	$(STRL_SRCS) \
	verrorf.c \
	xalloc.c \
	xf86allocscripi.c \
	xf86addrestolist.c \
	xf86drvmsg.c \
	xf86drvmsgverb.c \
	xf86errorf.c \
	xf86errorfverb.c \
	xf86getverb.c \
	xf86info.c \
	xf86msg.c \
	xf86msgverb.c \
	xf86opt.c \
	xf86screens.c \
	xf86servisinit.c \
	xf86verbose.c \
	$(srcdir)/../os-support/shared/sigiostubs.c

EXTRA_DIST = README dummylib.h
