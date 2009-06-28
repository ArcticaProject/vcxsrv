INCLUDES = 					\
	@KDRIVE_INCS@				\
	@KDRIVE_CFLAGS@				\
	-I$(srcdir)/../../../exa

if XV
    LIBXEPHYR_HOSTXV=libxephyr-hostxv.a
else
    LIBXEPHYR_HOSTXV=
endif

if XEPHYR_HAS_DRI
    LIBXEPHYR_HOSTDRI=libxephyr-hostdri.a
else
    LIBXEPHYR_HOSTDRI=
endif

noinst_LIBRARIES = libxephyr-hostx.a $(LIBXEPHYR_HOSTXV) $(LIBXEPHYR_HOSTDRI) libxephyr.a

bin_PROGRAMS = Xephyr

libxephyr_hostx_a_SOURCES =                             \
	hostx.c                                         \
	hostx.h

libxephyr_hostxv_a_INCLUDES = @XEPHYR_INCS@

if XV
libxephyr_hostxv_a_SOURCES=				\
	ephyrhostvideo.c \
	ephyrhostvideo.h
endif

if XEPHYR_HAS_DRI

libxephyr_hostdri_a_SOURCES= \
ephyrdriext.c	\
ephyrdriext.h	\
ephyrdri.c	\
ephyrdri.h	\
XF86dri.c	\
ephyrglxext.c	\
ephyrglxext.h	\
ephyrhostglx.c	\
ephyrhostglx.h	

libxephyr_hostdri_a_CFLAGS= \
-I$(top_srcdir)		\
@LIBDRM_CFLAGS@ 	\
@DRIPROTO_CFLAGS@

endif

libxephyr_a_SOURCES =	\
	ephyr.c		\
	ephyr_draw.c	\
	ephyrvideo.c	\
	os.c		\
	hostx.h         \
	ephyr.h		\
	ephyrlog.h

libxephyr_a_CFLAGS = \
-I$(top_srcdir) \
@LIBDRM_CFLAGS@

Xephyr_SOURCES = \
	ephyrinit.c

Xephyr_LDADD = 						\
	libxephyr.a					\
	libxephyr-hostx.a			        \
	$(LIBXEPHYR_HOSTXV)				\
	$(LIBXEPHYR_HOSTDRI)				\
	../../../exa/libexa.la				\
	@KDRIVE_LIBS@					\
        @XEPHYR_LIBS@ 					\
	@LIBDRM_LIBS@					\
	@XEPHYR_DRI_LIBS@

Xephyr_DEPENDENCIES =	\
	libxephyr.a					\
	libxephyr-hostx.a			        \
	$(LIBXEPHYR_HOSTXV)				\
	$(LIBXEPHYR_HOSTDRI)				\
	@KDRIVE_LOCAL_LIBS@

Xephyr_LDFLAGS = $(LD_EXPORT_SYMBOLS_FLAG)

relink:
	rm -f $(bin_PROGRAMS) && make $(bin_PROGRAMS)
