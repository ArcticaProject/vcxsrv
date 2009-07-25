INCLUDES = 			\
	@KDRIVE_INCS@		\
	@KDRIVE_CFLAGS@		\
	@XEPHYR_INCS@		\
	@XEPHYR_CFLAGS@		\
	@DRIPROTO_CFLAGS@	\
	-I$(top_srcdir)		\
	-I$(top_srcdir)/exa

if XV
LIBXEPHYR_HOSTXV=libxephyr-hostxv.a
endif

if DRI
LIBXEPHYR_HOSTDRI=libxephyr-hostdri.a
endif

noinst_LIBRARIES = libxephyr-hostx.a $(LIBXEPHYR_HOSTXV) $(LIBXEPHYR_HOSTDRI) libxephyr.a

bin_PROGRAMS = Xephyr

HOSTX_SRCS =			\
	hostx.c			\
	hostx.h

HOSTVIDEO_SRCS =		\
	ephyrvideo.c		\
	ephyrhostvideo.c	\
	ephyrhostvideo.h

HOSTDRI_SRCS =			\
	ephyrdriext.c		\
	ephyrdriext.h		\
	ephyrdri.c		\
	ephyrdri.h		\
	XF86dri.c		\
	xf86dri.h		\
	ephyrglxext.c		\
	ephyrglxext.h		\
	ephyrhostglx.c		\
	ephyrhostglx.h

XEPHYR_SRCS =			\
	ephyr.c			\
	ephyr.h			\
	ephyrlog.h		\
	ephyr_draw.c		\
	os.c

libxephyr_hostx_a_SOURCES = $(HOSTX_SRCS)

if XV
libxephyr_hostxv_a_SOURCES = $(HOSTVIDEO_SRCS)
endif

if DRI
libxephyr_hostdri_a_SOURCES = $(HOSTDRI_SRCS)
endif

libxephyr_a_SOURCES = $(XEPHYR_SRCS)

Xephyr_SOURCES = \
	ephyrinit.c

Xephyr_LDADD = 						\
	libxephyr.a					\
	libxephyr-hostx.a				\
	$(LIBXEPHYR_HOSTXV)				\
	$(LIBXEPHYR_HOSTDRI)				\
	$(top_builddir)/exa/libexa.la			\
	@KDRIVE_LIBS@					\
	@XEPHYR_LIBS@

Xephyr_DEPENDENCIES =	\
	libxephyr.a					\
	libxephyr-hostx.a				\
	$(LIBXEPHYR_HOSTXV)				\
	$(LIBXEPHYR_HOSTDRI)				\
	@KDRIVE_LOCAL_LIBS@

Xephyr_LDFLAGS = $(LD_EXPORT_SYMBOLS_FLAG)

relink:
	rm -f $(bin_PROGRAMS) && make $(bin_PROGRAMS)

MAN_SRCS = Xephyr.man.pre

appmandir = $(APP_MAN_DIR)
appman_DATA = Xephyr.$(APP_MAN_SUFFIX)

Xephyr.$(APP_MAN_SUFFIX): Xephyr.man
	-rm -f Xephyr.$(APP_MAN_SUFFIX)
	$(LN_S) Xephyr.man Xephyr.$(APP_MAN_SUFFIX)

include $(top_srcdir)/cpprules.in

CLEANFILES = $(appman_DATA) Xephyr.man

EXTRA_DIST = \
	$(HOSTVIDEO_SRCS) \
	$(HOSTDRI_SRCS) \
	$(MAN_SRCS)
