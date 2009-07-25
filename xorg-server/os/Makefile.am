noinst_LTLIBRARIES = libos.la

AM_CFLAGS = $(DIX_CFLAGS)

SECURERPC_SRCS = rpcauth.c
XDMCP_SRCS = xdmcp.c
STRLCAT_SRCS = strlcat.c strlcpy.c
XORG_SRCS = log.c

libos_la_SOURCES = 	\
	WaitFor.c	\
	access.c	\
	auth.c		\
	backtrace.c	\
	connection.c	\
	io.c		\
	mitauth.c	\
	oscolor.c	\
	osdep.h		\
	osinit.c	\
	utils.c		\
	strcasecmp.c	\
	strcasestr.c	\
	xdmauth.c	\
	xstrans.c	\
	xprintf.c	\
	$(XORG_SRCS)

if SECURE_RPC
libos_la_SOURCES += $(SECURERPC_SRCS)
endif

if XDMCP
libos_la_SOURCES += $(XDMCP_SRCS)
endif

if NEED_STRLCAT
libos_la_SOURCES += $(STRLCAT_SRCS)
endif

EXTRA_DIST = $(SECURERPC_SRCS) $(INTERNALMALLOC_SRCS) \
     $(XDMCP_SRCS) $(STRLCAT_SRCS)

if XSERVER_DTRACE
# Generate dtrace object code for probes in libos & libdix
dtrace.o: $(top_srcdir)/dix/Xserver.d $(am_libos_la_OBJECTS)
	$(DTRACE) -G -C -o $@ -s $(top_srcdir)/dix/Xserver.d .libs/*.o ../dix/.libs/*.o

noinst_PROGRAMS = os.O

os.O: dtrace.o $(am_libos_la_OBJECTS)
	ld -r -o $@ dtrace.o .libs/*.o
endif

os.c:
	touch $@

CLEANFILES = os.c
